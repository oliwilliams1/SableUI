#include <SableUI/window.h>
#include <SableUI/renderer.h>
#include <SableUI/memory.h>
#include <SableUI/textCache.h>
#include <SableUI/console.h>
#include <SableUI/drawable.h>
#include <SableUI/events.h>
#include <SableUI/panel.h>
#include <SableUI/texture.h>
#include <SableUI/utils.h>
#include <SableUI/element.h>
#include <SableUI/generated/resources.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
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

using namespace SableMemory;

static float DistToEdge(SableUI::BasePanel* node, SableUI::ivec2 p)
{
	SableUI::Rect r = node->rect;

	SableUI::PanelType parentType = (node->parent == nullptr) ? SableUI::PanelType::ROOTNODE : node->parent->type;

	switch (parentType)
	{
	case SableUI::PanelType::ROOTNODE:
	{
		float distLeft = p.x - r.x;
		float distRight = (r.x + r.w) - p.x;
		float distTop = p.y - r.y;
		float distBottom = (r.y + r.h) - p.y;

		return (std::max)(0.0f, (std::min)({ distLeft, distRight, distTop, distBottom }));
	}

	case SableUI::PanelType::HORIZONTAL:
	{
		float distRight = (r.x + r.w) - p.x;
		return (distRight < 0) ? 0 : distRight;
	}

	case SableUI::PanelType::VERTICAL:
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

	instance->m_renderer->Viewport(0, 0, width, height);

	if (width > 0 && height > 0)
	{
		instance->m_colourAttachment.CreateStorage(width, height, TextureFormat::RGBA8, TextureUsage::RenderTarget);
		instance->m_framebuffer.SetSize(width, height);
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
SableUI::Window::Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, int x, int y)
{
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	m_windowSize = ivec2(width, height);

	if (primary == nullptr)
	{
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	}
	else
	{
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, primary->m_window);
	}

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

	m_renderer = RendererBackend::Create(backend);

	m_renderer->SetBlending(true);
	m_renderer->SetBlendFunction(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
	m_renderer->Clear(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);

	if (width > 0 && height > 0)
	{
		m_colourAttachment.CreateStorage(m_windowSize.x, m_windowSize.y, TextureFormat::RGBA8, TextureUsage::RenderTarget);
		m_framebuffer.SetSize(m_windowSize.x, m_windowSize.y);
		m_framebuffer.AttachColour(&m_colourAttachment, 0);
		m_framebuffer.Bake();
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

	m_root = SB_new<SableUI::RootPanel>(m_renderer, width, height);
}

void SableUI::Window::MakeContextCurrent()
{
	if (m_window)
	{
		glfwMakeContextCurrent(m_window);
	}
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
			case PanelType::VERTICAL:
				cursorToSet = m_vResizeCursor;
				break;

			case PanelType::HORIZONTAL:
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

bool SableUI::Window::Update()
{
	if (IsMinimized())
		return !glfwWindowShouldClose(m_window);

	MakeContextCurrent();

	if (m_needsRefresh)
	{
		m_renderer->ClearDrawableStack();
		RecalculateNodes();
		RerenderAllNodes();
		m_needsRefresh = false;
	}

	m_root->PropagateEvents(ctx);
	m_root->PropagateComponentStateChanges();
	m_root->PropagatePostLayoutEvents(ctx);

	StepCachedTexturesCleaner();
	TextCacheFactory::CleanCache(m_renderer);

	HandleResize();

	ctx.mousePressed.reset();
	ctx.mouseReleased.reset();
	ctx.mouseDoubleClicked.reset();
	ctx.keyPressedEvent.reset();
	ctx.keyReleasedEvent.reset();
	ctx.scrollDelta = { 0, 0 };
	ctx.typedCharBuffer.clear();

	return !glfwWindowShouldClose(m_window);
}

void SableUI::Window::Draw()
{
	if (IsMinimized())
		return;

	MakeContextCurrent();

#ifdef _WIN32
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			exit(0);
	}
#endif

	bool needsRedraw = false;
	bool hasWindowSurfaceQueue = false;

	// check if any custom queue renders to window surface
	for (CustomTargetQueue* queue : m_customTargetQueues)
	{
		if (queue->target == &m_windowSurface)
		{
			hasWindowSurfaceQueue = true;
			break;
		}
	}

	// render main content to offscreen framebuffer
	if (m_renderer->isDirty() || m_needsStaticRedraw)
	{
		needsRedraw = true;

		m_renderer->BeginRenderPass(&m_framebuffer);
		m_renderer->Draw(&m_framebuffer);
		m_renderer->EndRenderPass();
	}

	// blit the base framebuffer to screen if we need it
	if (needsRedraw || hasWindowSurfaceQueue)
		m_renderer->BlitToScreen(&m_framebuffer);

	// execute custom queues
	if (!m_customTargetQueues.empty())
	{
		for (CustomTargetQueue* queue : m_customTargetQueues)
		{
			if (!queue->drawables.empty() || queue->root)
			{
				for (DrawableBase* dr : queue->drawables)
					m_renderer->AddToDrawStack(dr);

				if (queue->root)
				{
					queue->root->LayoutChildren();
					queue->root->Render();
				}

				// rf rendering to window surface, render directly to back buffer (framebuffer 0)
				if (queue->target == &m_windowSurface)
				{
					needsRedraw = true;
					m_renderer->BeginRenderPass(&m_windowSurface);
					m_renderer->Draw(&m_windowSurface);
					m_renderer->EndRenderPass();
				}
				else
				{
					// render to custom framebuffer
					m_renderer->BeginRenderPass(queue->target);
					m_renderer->Draw(queue->target);
					m_renderer->EndRenderPass();
				}
			}
		}
	}

	// swap buffers if anything was drawn
	if (needsRedraw)
	{
		m_renderer->CheckErrors();
		glfwSwapBuffers(m_window);
		m_needsStaticRedraw = false;
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
	m_renderer->ClearDrawableStack();

	m_root->Render();

	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	m_root->Recalculate();
}

SableUI::ElementInfo SableUI::Window::GetElementInfoById(const SableString& id)
{
	Element* el = m_root->GetElementById(id);
	if (el)
	{
		const ElementInfo& elInfo = el->GetInfo();
		return elInfo;
	}
	else
		return ElementInfo{};
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
static void FixWidth(SableUI::BasePanel* panel)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::BASE)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixWidth(child);

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.w < child->minBounds.x)
		{
			child->rect.w = child->minBounds.x;
			child->rect.wType = SableUI::RectType::Fixed;
			resized = true;
		}

		if (child->maxBounds.x > 0 && child->rect.w > child->maxBounds.x)
		{
			int excess = child->rect.w - child->maxBounds.x;
			child->rect.w = child->maxBounds.x;
			child->rect.wType = SableUI::RectType::Fixed;
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

			if (child->rect.wType == SableUI::RectType::Fixed ||
				child->rect.wType == SableUI::RectType::Fill)
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
					panels[i]->rect.wType = SableUI::RectType::Fixed;

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
			if (child->rect.wType == SableUI::RectType::Fixed)
			{
				fixedWidthSum += child->rect.w;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->rect.wType != SableUI::RectType::Fixed)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.w = fixedWidthSum;
			panel->rect.wType = SableUI::RectType::Fixed;
		}

		panel->CalculateScales();
		panel->CalculatePositions();
	}
}

static void FixHeight(SableUI::BasePanel* panel)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::BASE)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixHeight(child);

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.h < child->minBounds.y)
		{
			child->rect.h = child->minBounds.y;
			child->rect.hType = SableUI::RectType::Fixed;
			resized = true;
		}

		if (child->maxBounds.y > 0 && child->rect.h > child->maxBounds.y)
		{
			int excess = child->rect.h - child->maxBounds.y;
			child->rect.h = child->maxBounds.y;
			child->rect.hType = SableUI::RectType::Fixed;
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

			if (child->rect.hType == SableUI::RectType::Fixed ||
				child->rect.hType == SableUI::RectType::Fill)
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
					panels[i]->rect.hType = SableUI::RectType::Fixed;

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
			if (child->rect.hType == SableUI::RectType::Fixed)
			{
				fixedHeightSum += child->rect.h;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->rect.hType != SableUI::RectType::Fixed)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.h = fixedHeightSum;
			panel->rect.hType = SableUI::RectType::Fixed;
		}

		panel->CalculateScales();
		panel->CalculatePositions();
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
		case PanelType::HORIZONTAL:
			state.currentEdgeType = EdgeType::EW_EDGE;
			break;
		case PanelType::VERTICAL:
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

		state.selectedPanel->rect.wType = SableUI::RectType::Fixed;
		state.selectedPanel->rect.w = width;

		state.olderSiblingNode->rect.wType = SableUI::RectType::Fill;

		FixWidth(state.selectedPanel);
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

		state.selectedPanel->rect.hType = SableUI::RectType::Fixed;
		state.selectedPanel->rect.h = height;

		state.olderSiblingNode->rect.hType = SableUI::RectType::Fill;

		FixHeight(state.selectedPanel);
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
	m_renderer->ClearDrawableStack();
	m_needsRefresh = true;

	oldPos = pos;
}

SableUI::Window::~Window()
{
	if (m_window)
		MakeContextCurrent();

	SB_delete(m_root);
	DestroyDrawables();

	TextCacheFactory::ShutdownFactory(m_renderer);

	SB_delete(m_renderer);

	if (m_window)
		glfwDestroyWindow(m_window);
}


void SableUI::SableUI_Window_Initalise_GLFW()
{
	if (!glfwInit())
		SableUI_Runtime_Error("Could not initialize GLFW");
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