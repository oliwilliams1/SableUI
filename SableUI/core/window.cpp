#include <SableUI/core/window.h>
#include <SableUI/SableUI.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/text_cache.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/events.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/texture.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/element.h>
#include <SableUI/generated/resources.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <string>
#include <vector>

#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")
#include <windows.h>
#include <dwmapi.h>
#endif
#include <SableUI/core/component.h>

using namespace SableMemory;

static float DistToEdge(SableUI::BasePanel* node, SableUI::ivec2 p)
{
	SableUI::Rect r = node->rect;

	SableUI::PanelType parentType = (node->parent == nullptr) ? SableUI::PanelType::Root : node->parent->type;

	switch (parentType)
	{
	case SableUI::PanelType::Root:
	{
		float distLeft = p.x - r.x;
		float distRight = (r.x + r.w) - p.x;
		float distTop = p.y - r.y;
		float distBottom = (r.y + r.h) - p.y;

		return (std::max)(0.0f, (std::min)({ distLeft, distRight, distTop, distBottom }));
	}

	case SableUI::PanelType::HorizontalSplitter:
	{
		float distRight = (r.x + r.w) - p.x;
		return (distRight < 0) ? 0 : distRight;
	}

	case SableUI::PanelType::VerticalSplitter:
	{
		float distBottom = (r.y + r.h) - p.y;
		return (distBottom < 0) ? 0 : distBottom;
	}

	default:
		return 0.0f;
	}
}

// ============================================================================
// Callbacks
// ============================================================================
void SableUI::Window::MousePosCallback(GLFWwindow* window, double x, double y)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	ivec2 oldPos = instance->ctx.mousePos;

	instance->ctx.mousePos = { static_cast<int>(x), static_cast<int>(y) };
	instance->ctx.mouseDelta = instance->ctx.mousePos - oldPos;
}

void SableUI::Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	if (button < 0 || button >= SABLE_MAX_MOUSE_BUTTONS) return;

	auto& ctx = instance->ctx;

	if (action == GLFW_PRESS)
	{
		ctx.mouseDown.set(button, true);
		ctx.mousePressed.set(button, true);

		double currentTime = glfwGetTime();
		double timeSinceLastClick = currentTime - instance->m_lastClickTime[button];

		ivec2 currentPos = instance->ctx.mousePos;
		ivec2 lastPos = instance->m_lastClickPos[button];
		int distanceMoved = std::abs(currentPos.x - lastPos.x) + std::abs(currentPos.y - lastPos.y);

		if (timeSinceLastClick < instance->DOUBLE_CLICK_TIME &&
			distanceMoved < instance->DOUBLE_CLICK_MAX_DIST)
		{
			ctx.mouseDoubleClicked.set(button, true);
			instance->m_lastClickTime[button] = 0.0;
		}
		else
		{
			instance->m_lastClickTime[button] = currentTime;
			instance->m_lastClickPos[button] = currentPos;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		ctx.mouseDown.set(button, false);
		ctx.mouseReleased.set(button, true);
	}
}

void SableUI::Window::ResizeCallback(GLFWwindow* window, int width, int height)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	instance->m_windowSize = ivec2(width, height);

	if (width <= 0 || height <= 0)
	{
		instance->m_isMinimized = true;
		return;
	}

	instance->m_isMinimized = false;
	instance->MakeContextCurrent();

	instance->m_baseRenderer->Viewport(0, 0, width, height);
	if (width > 0 && height > 0)
	{
		instance->m_baseColourAttachment.CreateStorage(width, height, TextureFormat::RGBA8, TextureUsage::RenderTarget);
		instance->m_baseFramebuffer.SetSize(width, height);
		instance->m_windowSurface.SetSize(width, height);
	}

	instance->m_root->Resize(width, height);
	instance->RecalculateNodes();
	instance->RerenderAllNodes();
	instance->m_needsStaticRedraw = true;
}

void SableUI::Window::WindowRefreshCallback(GLFWwindow* window)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	instance->m_needsStaticRedraw = true;
	instance->m_needsRefresh = true;
}

void SableUI::Window::ScrollCallback(GLFWwindow* window, double x, double y)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	instance->ctx.scrollDelta = { static_cast<float>(x), static_cast<float>(y) };
}

void SableUI::Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (0 > key || key > SABLE_MAX_KEYS) return;

	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		instance->ctx.keyPressedEvent.set(key, true);
		instance->ctx.isKeyDown.set(key, true);
	}

	if (action == GLFW_RELEASE)
	{
		instance->ctx.keyReleasedEvent.set(key, true);
		instance->ctx.isKeyDown.set(key, false);
	}
}

void SableUI::Window::CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	if (codepoint > 31 && codepoint < 127)
	{
		instance->ctx.typedCharBuffer.push_back(codepoint);
	}
}

// ============================================================================
// Window
// ============================================================================
SableUI::Window::Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, const WindowInitInfo& info)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	m_windowSize = ivec2(width, height);

	if (info.posX > 0)
		glfwWindowHint(GLFW_POSITION_X, info.posX);
	if (info.posY > 0)
		glfwWindowHint(GLFW_POSITION_Y, info.posY);

	glfwWindowHint(GLFW_DECORATED, info.decorated);
	glfwWindowHint(GLFW_RESIZABLE, info.resisable);
	glfwWindowHint(GLFW_FLOATING, info.floating);
	glfwWindowHint(GLFW_MAXIMIZED, info.maximised);

	if (primary == nullptr)
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	else
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, primary->m_window);

	MakeContextCurrent();
	
	int iconWidth = 0, iconHeight = 0, iconChannels = 0;
	unsigned char* iconData = stbi_load_from_memory(sableui_64x_png_data, sableui_64x_png_size, &iconWidth, &iconHeight, &iconChannels, 4);
	GLFWimage image{};
	image.width = iconWidth;
	image.height = iconHeight;
	image.pixels = iconData;

	glfwSetWindowIcon(m_window, 1, &image);
	stbi_image_free(iconData);

	glfwSwapInterval(1);
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));

#ifdef _WIN32
	// Enable immersive dark mode on windows via api 
	HWND hwnd = FindWindowA(NULL, title.c_str());

	BOOL dark_mode = true;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));

	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);
#endif

	m_baseRenderer = RendererBackend::Create(backend);
	m_baseRenderer->SetBlending(true);
	m_baseRenderer->SetBlendFunction(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
	m_baseRenderer->Clear(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);

	SetupContextBindings(m_baseRenderer);

	if (width > 0 && height > 0)
	{
		m_baseColourAttachment.CreateStorage(m_windowSize.x, m_windowSize.y, TextureFormat::RGBA8, TextureUsage::RenderTarget);
		m_baseFramebuffer.SetSize(m_windowSize.x, m_windowSize.y);
		m_baseFramebuffer.AttachColour(&m_baseColourAttachment, 0);
		m_baseFramebuffer.Bake();
	}

	m_windowSurface.SetIsWindowSurface(true);
	m_windowSurface.SetSize(m_windowSize.x, m_windowSize.y);

	if (m_root != nullptr)
	{
		SableUI_Error("Root node already created!");
		return;
	}

	m_arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_hResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_vResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

	glfwSetCursorPosCallback(m_window, MousePosCallback);
	glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
	glfwSetWindowSizeCallback(m_window, ResizeCallback);
	glfwSetWindowRefreshCallback(m_window, WindowRefreshCallback);
	glfwSetScrollCallback(m_window, ScrollCallback);
	glfwSetKeyCallback(m_window, KeyCallback);
	glfwSetCharCallback(m_window, CharCallback);

	m_root = SB_new<SableUI::RootPanel>(m_baseRenderer, width, height);
}

void SableUI::Window::MakeContextCurrent()
{
	if (!m_window)
		return;

	if (glfwGetCurrentContext() == m_window)
		return;

	glfwMakeContextCurrent(m_window);
}

bool SableUI::Window::IsMinimized() const
{
	return m_windowSize.x <= 0 || m_windowSize.y <= 0;
}

void SableUI::Window::HandleResize()
{
	// static for multiple calls on one resize event (lifetime of static is until mouse up)
	static bool resCalled = false;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (DistToEdge(m_root, ctx.mousePos) > 5.0f)
	{
		cursorToSet = CheckResize(m_root, &resCalled, false);
	}

	if (m_currentCursor != cursorToSet && !m_resizing)
	{
		glfwSetCursor(m_window, cursorToSet);
		m_currentCursor = cursorToSet;
	}

	if (m_resizing)
	{
		if (!resCalled && m_currentCursor != m_arrowCursor)
		{
			Resize(ctx.mousePos);
		}
		else
		{
			resCalled = false;
		}
		if (!IsMouseDown(ctx, SABLE_MOUSE_BUTTON_LEFT))
		{
			m_resizing = false;
		}
	}
}

GLFWcursor* SableUI::Window::CheckResize(BasePanel* node, bool* resCalled, bool p_isLastChild)
{
	if (node == nullptr) return m_arrowCursor;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (RectBoundingBox(node->rect, ctx.mousePos) 
		&& ctx.mousePos.x != 0 && ctx.mousePos.y != 0 
		&& !p_isLastChild)
	{
		float d1 = DistToEdge(node, ctx.mousePos);

		if (node->parent != nullptr && d1 < 5.0f)
		{
			switch (node->parent->type)
			{
			case PanelType::VerticalSplitter:
				cursorToSet = m_vResizeCursor;
				break;

			case PanelType::HorizontalSplitter:
				cursorToSet = m_hResizeCursor;
				break;
			default:
				break;
			}

			if (!m_resizing && IsMouseDown(ctx, SABLE_MOUSE_BUTTON_LEFT))
			{
				*resCalled = true;
				Resize(ctx.mousePos, node);
				m_resizing = true;
			}
		}
	}

	for (BasePanel* child : node->children)
	{
		bool isLastChild = child == node->children.back();
		GLFWcursor* childCursor = CheckResize(child, resCalled, isLastChild);
		if (childCursor != m_arrowCursor)
		{
			cursorToSet = childCursor;
		}
	}

	if (!IsMouseDown(ctx, SABLE_MOUSE_BUTTON_LEFT))
		m_resizing = false;

	return cursorToSet;
}

bool SableUI::Window::Update(const std::unordered_set<TimerHandle>& firedTimers)
{
	if (IsMinimized())
		return !glfwWindowShouldClose(m_window);

	SetContext(this);

	AsyncTextureLoader::GetInstance().ProcessCompletedLoads();

	if (m_needsRefresh)
	{
		RecalculateNodes();
		RerenderAllNodes();
		m_needsRefresh = false;
	}

	ctx.firedTimers = firedTimers;

	CommandBuffer& cmd = m_baseRenderer->GetCommandBuffer();
	ContextResources& contextResources = GetContextResources(m_baseRenderer);

	// regular panels
	m_root->DistributeEvents(ctx);
	bool dirty = m_root->UpdateComponents(cmd, &m_baseFramebuffer, contextResources);
	if (dirty)
	{
		m_root->Render(cmd, &m_baseFramebuffer, contextResources);
		m_needsStaticRedraw = true;
	}
	m_root->PostLayoutUpdate(ctx);

	StepCachedTexturesCleaner();
	TextCacheFactory::CleanCache(m_baseRenderer);
	HandleResize();

	ctx.mousePressed.reset();
	ctx.mouseReleased.reset();
	ctx.mouseDoubleClicked.reset();
	ctx.keyPressedEvent.reset();
	ctx.keyReleasedEvent.reset();
	ctx.scrollDelta = { 0, 0 };
	ctx.typedCharBuffer.clear();
	ctx.firedTimers.clear();

	return !glfwWindowShouldClose(m_window);
}

void SableUI::Window::Draw()
{
	if (IsMinimized())
		return;

	SetContext(this);
	MakeContextCurrent();

	bool baseLayerDirty = m_baseRenderer->isDirty() || m_needsStaticRedraw;
	bool anyFloatingPanelDirty = false;

	//for (const auto& pair : m_floatingPanels)
	//{
	//	if (pair.second->IsDirty())
	//	{
	//		anyFloatingPanelDirty = true;
	//		break;
	//	}
	//}

	if (baseLayerDirty || anyFloatingPanelDirty || !m_customTargetQueues.empty())
		m_syncFrames = 2;

	if (m_syncFrames > 0)
	{
		if (baseLayerDirty)
		{
			m_baseRenderer->BeginRenderPass(&m_baseFramebuffer);
			m_baseRenderer->ExecuteCommandBuffer();
			m_baseRenderer->EndRenderPass();
		}

		//for (const auto& pair : m_floatingPanels)
		//	if (pair.second->IsDirty())
		//		pair.second->Render();

		m_baseRenderer->BlitToScreen(&m_baseFramebuffer);

		//for (const auto& pair : m_floatingPanels)
		//{
		//	FloatingPanel* panel = pair.second;
		//	GpuFramebuffer* panelFBO = const_cast<GpuFramebuffer*>(panel->GetFramebuffer());

		//	Rect sourceRect = { 0, 0, panel->rect.w, panel->rect.h };
		//	Rect destRect = panel->rect;
		//	destRect.y = m_windowSize.h - destRect.y - destRect.h;

		//	m_baseRenderer->DrawToScreen(panelFBO, sourceRect, destRect, m_windowSize);
		//}

		//for (CustomTargetQueue* queue : m_customTargetQueues)
		//{
		//	if (!queue->drawables.empty())
		//	{
		//		for (DrawableBase* dr : queue->drawables)
		//			m_baseRenderer->AddToDrawStack(dr);

		//		GpuFramebuffer* target = (queue->target == &m_windowSurface) ? &m_windowSurface : queue->target;
		//		m_baseRenderer->BeginRenderPass(target);
		//		m_baseRenderer->Draw(target);
		//		m_baseRenderer->EndRenderPass();
		//	}
		//}

		m_baseRenderer->CheckErrors();
		glfwSwapBuffers(m_window);
		m_needsStaticRedraw = false;
		m_syncFrames--;
		m_baseRenderer->GetCommandBuffer().Reset();
	}
}

void SableUI::Window::SetTitleBar(const SableString& title)
{
	glfwSetWindowTitle(m_window, std::string(title).c_str());
}

SableUI::RootPanel* SableUI::Window::GetRoot()
{
	return m_root;
}

void SableUI::Window::RerenderAllNodes()
{
	m_baseRenderer->ResetCommandBuffer();
	CommandBuffer& cmd = m_baseRenderer->GetCommandBuffer();
	ContextResources& contextResources = GetContextResources(m_baseRenderer);

	m_root->Render(cmd, &m_baseFramebuffer, contextResources);

	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	CommandBuffer& cmd = m_baseRenderer->GetCommandBuffer();
	ContextResources& contextResources = GetContextResources(m_baseRenderer);
	m_root->Recalculate(cmd, &m_baseFramebuffer, contextResources);
}

SableString SableUI::Window::GetClipboardContent()
{
	std::string utf8str = glfwGetClipboardString(m_window);
	return SableString(utf8str);
}

void SableUI::Window::SetClipboardContent(const SableString& content)
{
	std::string utf8str = content.to_utf8();
	glfwSetClipboardString(m_window, utf8str.c_str());
}

void SableUI::Window::SubmitCustomQueue(CustomTargetQueue* queue)
{
	if (std::find(m_customTargetQueues.begin(), m_customTargetQueues.end(), queue)
		== m_customTargetQueues.end())
	{
		m_customTargetQueues.push_back(queue);
	}
	else
	{
		SableUI_Runtime_Error("Custom target queue already exists");
	}
}

void SableUI::Window::RemoveQueueReference(CustomTargetQueue* reference)
{
	if (!reference->window || !reference->target) return;

	for (int i = 0; i < m_customTargetQueues.size(); i++)
	{
		if (m_customTargetQueues[i] == reference)
		{
			m_customTargetQueues.erase(m_customTargetQueues.begin() + i);
			break;
		}
	}
}

// ============================================================================
// Node calculations
// ============================================================================
static void FixWidth(SableUI::BasePanel* panel, SableUI::RendererBackend* renderer, const SableUI::GpuFramebuffer* fbo)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::Base)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixWidth(child, renderer, fbo);

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.w < child->minBounds.x)
		{
			child->rect.w = child->minBounds.x;
			child->wType = SableUI::RectType::Fixed;
			resized = true;
		}

		if (child->maxBounds.x > 0 && child->rect.w > child->maxBounds.x)
		{
			int excess = child->rect.w - child->maxBounds.x;
			child->rect.w = child->maxBounds.x;
			child->wType = SableUI::RectType::Fixed;
			deficit += excess;
			resized = true;
		}
	}

	if (deficit > 0)
	{
		std::vector<SableUI::BasePanel*> panels;

		for (size_t i = 0; i < panel->children.size(); i++)
		{
			SableUI::BasePanel* child = panel->children[i];

			if (child->maxBounds.x > 0 && child->rect.w >= child->maxBounds.x)
				continue;

			if (child->wType == SableUI::RectType::Fixed ||
				child->wType == SableUI::RectType::Fill)
			{
				panels.push_back(child);
			}
		}

		if (!panels.empty())
		{
			int deficitPerPanel = deficit / panels.size();
			int leftoverDeficit = deficit % panels.size();

			for (size_t i = 0; i < panels.size(); i++)
			{
				int additionalWidth = deficitPerPanel + (i < leftoverDeficit ? 1 : 0);
				panels[i]->rect.w += additionalWidth;

				if (panels[i]->maxBounds.x > 0 &&
					panels[i]->rect.w > panels[i]->maxBounds.x)
				{
					int overflow = panels[i]->rect.w - panels[i]->maxBounds.x;
					panels[i]->rect.w = panels[i]->maxBounds.x;
					panels[i]->wType = SableUI::RectType::Fixed;

					if (i + 1 < panels.size())
					{
						leftoverDeficit += overflow;
					}
				}
			}
		}
	}

	if (resized)
	{
		int fixedWidthSum = 0;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->wType == SableUI::RectType::Fixed)
			{
				fixedWidthSum += child->rect.w;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->wType != SableUI::RectType::Fixed)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.w = fixedWidthSum;
			panel->wType = SableUI::RectType::Fixed;
		}

		SableUI::CommandBuffer& cmd = renderer->GetCommandBuffer();
		SableUI::ContextResources& ctx = SableUI::GetContextResources(renderer);

		panel->CalculateScales();
		panel->CalculatePositions(cmd, fbo, ctx);
	}
}

static void FixHeight(SableUI::BasePanel* panel, SableUI::RendererBackend* renderer, const SableUI::GpuFramebuffer* fbo)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::Base)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixHeight(child, renderer, fbo);

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.h < child->minBounds.y)
		{
			child->rect.h = child->minBounds.y;
			child->hType = SableUI::RectType::Fixed;
			resized = true;
		}

		if (child->maxBounds.y > 0 && child->rect.h > child->maxBounds.y)
		{
			int excess = child->rect.h - child->maxBounds.y;
			child->rect.h = child->maxBounds.y;
			child->hType = SableUI::RectType::Fixed;
			deficit += excess;
			resized = true;
		}
	}

	if (deficit > 0)
	{
		std::vector<SableUI::BasePanel*> panels;

		for (size_t i = 0; i < panel->children.size(); i++)
		{
			SableUI::BasePanel* child = panel->children[i];

			if (child->maxBounds.y > 0 && child->rect.h >= child->maxBounds.y)
				continue;

			if (child->hType == SableUI::RectType::Fixed ||
				child->hType == SableUI::RectType::Fill)
			{
				panels.push_back(child);
			}
		}

		if (!panels.empty())
		{
			int deficitPerPanel = deficit / panels.size();
			int leftoverDeficit = deficit % panels.size();

			for (size_t i = 0; i < panels.size(); i++)
			{
				int additionalHeight = deficitPerPanel + (i < leftoverDeficit ? 1 : 0);
				panels[i]->rect.h += additionalHeight;

				if (panels[i]->maxBounds.y > 0 &&
					panels[i]->rect.h > panels[i]->maxBounds.y)
				{
					int overflow = panels[i]->rect.h - panels[i]->maxBounds.y;
					panels[i]->rect.h = panels[i]->maxBounds.y;
					panels[i]->hType = SableUI::RectType::Fixed;

					if (i + 1 < panels.size())
					{
						leftoverDeficit += overflow;
					}
				}
			}
		}
	}

	if (resized)
	{
		int fixedHeightSum = 0;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->hType == SableUI::RectType::Fixed)
			{
				fixedHeightSum += child->rect.h;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->hType != SableUI::RectType::Fixed)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.h = fixedHeightSum;
			panel->hType = SableUI::RectType::Fixed;
		}

		SableUI::CommandBuffer& cmd = renderer->GetCommandBuffer();
		SableUI::ContextResources& ctx = SableUI::GetContextResources(renderer);
		panel->CalculateScales();
		panel->CalculatePositions(cmd, fbo, ctx);
	}
}

void SableUI::Window::ResizeStep(SableUI::ivec2 deltaPos, SableUI::BasePanel* panel, SableUI::BasePanel* root)
{
	auto& state = m_resizeState;

	if (panel != nullptr)
	{
		state.selectedPanel = panel;
		state.oldPanelRect = panel->rect;
		state.prevPos = { 0,0 };
		state.totalDelta = { 0,0 };

		if (panel->parent == nullptr)
		{
			state.olderSiblingNode = nullptr;
			state.olderSiblingOldRect = { 0,0,0,0 };
			return;
		}

		auto& siblings = panel->parent->children;
		auto it = std::find(siblings.begin(), siblings.end(), state.selectedPanel);
		if (it != siblings.end() && std::next(it) != siblings.end())
		{
			state.olderSiblingNode = *std::next(it);
			state.olderSiblingOldRect = state.olderSiblingNode->rect;
		}
		else
		{
			state.olderSiblingNode = nullptr;
			state.olderSiblingOldRect = { 0,0,0,0 };
			return;
		}
		
		switch (panel->parent->type)
		{
		case PanelType::HorizontalSplitter:
			state.currentEdgeType = EdgeType::EW_EDGE;
			break;
		case PanelType::VerticalSplitter:
			state.currentEdgeType = EdgeType::NS_EDGE;
			break;
		default:
			return;
		}

		return;
	}

	state.totalDelta = state.totalDelta + deltaPos;

	{
		SableUI::ivec2 currentPos = state.prevPos + deltaPos;
		SableUI::ivec2 dPos = state.prevPos - currentPos;
		if (dPos.x == 0 && dPos.y == 0) return;
		state.prevPos = currentPos;
	}

	if (state.selectedPanel == nullptr)
	{
		SableUI_Log("No node selected");
		return;
	}

	switch (state.currentEdgeType)
	{
	case SableUI::EdgeType::EW_EDGE:
	{
		int width = state.oldPanelRect.w + state.totalDelta.x;
		width = (std::max)(width, state.selectedPanel->minBounds.x);

		int maxWidth = state.selectedPanel->parent->rect.w - state.olderSiblingNode->minBounds.x;
		width = (std::min)(width, maxWidth);

		int newOlderSiblingWidth = state.olderSiblingOldRect.w - (width - state.oldPanelRect.w);
		newOlderSiblingWidth = (std::max)(newOlderSiblingWidth, state.olderSiblingNode->minBounds.x);

		state.selectedPanel->wType = SableUI::RectType::Fixed;
		state.selectedPanel->rect.w = width;

		state.olderSiblingNode->wType = SableUI::RectType::Fill;

		FixWidth(state.selectedPanel, m_baseRenderer, &m_baseFramebuffer);
		break;
	}
	case SableUI::EdgeType::NS_EDGE:
	{
		int height = state.oldPanelRect.h + state.totalDelta.y;
		height = (std::max)(height, state.selectedPanel->minBounds.y);

		int maxHeight = state.selectedPanel->parent->rect.h - state.olderSiblingNode->minBounds.y;
		height = (std::min)(height, maxHeight);

		int newOlderSiblingHeight = state.olderSiblingOldRect.h - (height - state.oldPanelRect.h);
		newOlderSiblingHeight = (std::max)(newOlderSiblingHeight, state.olderSiblingNode->minBounds.y);

		state.selectedPanel->hType = SableUI::RectType::Fixed;
		state.selectedPanel->rect.h = height;

		state.olderSiblingNode->hType = SableUI::RectType::Fill;

		FixHeight(state.selectedPanel, m_baseRenderer, &m_baseFramebuffer);
		break;
	}
	default:
		break;
	}

	state.selectedPanel->parent->CalculateScales();
}

void SableUI::Window::Resize(SableUI::ivec2 pos, SableUI::BasePanel* panel)
{
	const int threshold = 1;
	static SableUI::ivec2 oldPos = { 0, 0 };
	static SableUI::ivec2 pendingDelta = { 0, 0 };

	SableUI::ivec2 deltaPos = pos - oldPos;
	pendingDelta = pendingDelta + deltaPos;

	while (std::abs(pendingDelta.x) > threshold || std::abs(pendingDelta.y) > threshold)
	{
		SableUI::ivec2 stepDelta = { 0, 0 };

		if (std::abs(pendingDelta.x) > threshold)
		{
			stepDelta.x = (pendingDelta.x > 0) ? threshold : -threshold;
		}
		else
		{
			stepDelta.x = pendingDelta.x;
		}

		if (std::abs(pendingDelta.y) > threshold)
		{
			stepDelta.y = (pendingDelta.y > 0) ? threshold : -threshold;
		}
		else
		{
			stepDelta.y = pendingDelta.y;
		}

		ResizeStep(stepDelta, panel, m_root);

		pendingDelta = pendingDelta - stepDelta;
	}

	// Clear accumulated draw calls from resize steps
	m_baseRenderer->ResetCommandBuffer();
	m_needsRefresh = true;

	oldPos = pos;
}

SableUI::Window::~Window()
{
	if (m_window)
		MakeContextCurrent();

	SB_delete(m_root);
	DestroyContextResources(m_baseRenderer);

	TextCacheFactory::ShutdownFactory(m_baseRenderer);

	SB_delete(m_baseRenderer);

	if (m_window)
		glfwDestroyWindow(m_window);
}

void SableUI::SableUI_Window_Initialise_GLFW()
{
	if (!glfwInit())
		SableUI_Runtime_Error("Could not initialise GLFW");
}

void SableUI::SableUI_Window_Terminate_GLFW()
{
	glfwTerminate();
}

void SableUI::SableUI_Window_PollEvents_GLFW()
{
	glfwPollEvents();
}

void SableUI::SableUI_Window_WaitEvents_GLFW()
{
	glfwWaitEvents();
}

void SableUI::SableUI_Window_PostEmptyEvent_GLFW()
{
	glfwPostEmptyEvent();
}

void SableUI::SableUI_Window_WaitEventsTimeout_GLFW(double timeout)
{
	glfwWaitEventsTimeout(timeout);
}

void* SableUI::GetCurrentContext_voidType()
{
	return static_cast<void*>(glfwGetCurrentContext());
}
