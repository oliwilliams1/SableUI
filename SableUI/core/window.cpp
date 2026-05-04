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
#include <SableUI/generated/resources.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/types/floating_panel_types.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <bitset>
#include <iterator>
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")
#include <windows.h>
#include <dwmapi.h>
#endif

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

	instance->m_renderer->ExecuteCommandBuffer(instance->m_mainCommandBuffer);
	instance->m_mainCommandBuffer.Reset();

	instance->m_mainCommandBuffer.SetViewport(0, 0, width, height);
	if (width > 0 && height > 0)
	{
		instance->m_mainCommandBuffer.CreateStorageTexture2D(instance->m_colourAttachment, width, height, TextureFormat::RGBA8, TextureUsage::RenderTarget);
		instance->m_mainCommandBuffer.SetFramebufferSize(instance->m_framebuffer, width, height);
		instance->m_mainCommandBuffer.SetFramebufferSize(instance->m_windowSurface, width, height);
	}

	instance->m_root->Resize(width, height);
	instance->RecalculateNodes();
	instance->RerenderAllNodes();
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
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
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
	// Enable immersive dark mode on windows 
	HWND hwnd = FindWindowA(NULL, title.c_str());

	BOOL dark_mode = true;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));

	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);
#endif

	m_renderer = RendererBackend::Create(m_mainCommandBuffer, backend);
	m_mainCommandBuffer.SetBlendState(true, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
	m_mainCommandBuffer.Clear(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);

	m_colourAttachment = m_mainCommandBuffer.CreateTexture2D(width, height, TextureFormat::RGBA8, TextureUsage::RenderTarget);
	m_framebuffer = m_mainCommandBuffer.CreateFramebuffer(width, height, false);
	m_windowSurface = m_mainCommandBuffer.CreateFramebuffer(width, height, true);

	if (width > 0 && height > 0)
	{
		m_mainCommandBuffer.AttachColourTexture(m_framebuffer, m_colourAttachment, 0);
		m_mainCommandBuffer.BakeFramebuffer(m_framebuffer);
	}

	m_mainCommandBuffer.SetFramebufferSize(m_windowSurface, m_windowSize.x, m_windowSize.y);
	m_mainCommandBuffer.BeginRenderPass(m_framebuffer);

	m_compositeCommandBuffer = m_renderer->CreateSecondaryCommandBuffer();

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

void SableUI::Window::UnregisterFloatingPanel(int id)
{
	if (!m_floatingPanels.contains(id))
	{
		SableUI_Error("UnregisterFloatingPanel() called with a non-existent id");
	}

	FloatingPanelEntry& entry = m_floatingPanels[id];

	if (entry.texture.IsValid())
		m_mainCommandBuffer.DestroyTexture(entry.texture);
	if (entry.framebuffer.IsValid())
		m_mainCommandBuffer.DestroyFramebuffer(entry.framebuffer);

	m_floatingPanels.erase(id);
}

void SableUI::Window::RegisterFloatingPanel(int id, FloatingPanelBase* panel)
{
	if (m_floatingPanels.contains(id))
	{
		SableUI_Error("Floating panel with id %d already registered", id);
	}

	FloatingPanelEntry entry;
	entry.panel = panel;
	m_floatingPanels[id] = std::move(entry);
}

void SableUI::Window::ReassociateFloatingPanel(int id, FloatingPanelBase* panel)
{
	if (m_floatingPanels.contains(id))
		m_floatingPanels[id].panel = panel;
	else
		SableUI_Error("ReassociateFloatingPanel() called with a non-existent id");
}

void SableUI::Window::BuildCompositeCommandBuffer()
{
	ContextResources& ctxRes = GetContextResources(m_renderer);
	GlobalResources& globalRes = GetGlobalResources();

	m_compositeCommandBuffer.BeginRenderPass(m_framebuffer);
	m_compositeCommandBuffer.SetBlendState(true, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
	m_compositeCommandBuffer.SetPipeline(PipelineType::Image);
	m_compositeCommandBuffer.BindUniformBuffer(static_cast<uint32_t>(UboBinding::Rect), globalRes.ubo_rect);

	for (auto& entry : m_floatingPanels)
	{
		FloatingPanelEntry& fpEntry = entry.second;
		if (!fpEntry.panel->IsOpen()) continue;

		float height = ((fpEntry.size.y / static_cast<float>(m_windowSize.y)) * 2.0f);

		RectDrawData data{};
		data.rect[0] = (fpEntry.pos.x / static_cast<float>(m_windowSize.x)) * 2.0f - 1.0f;
		data.rect[1] = -((fpEntry.pos.y / static_cast<float>(m_windowSize.y)) * 2.0f - 1.0f) - height;
		data.rect[2] = (fpEntry.size.x / static_cast<float>(m_windowSize.x)) * 2.0f;
		data.rect[3] = height;
		data.realRect[0] = static_cast<float>(fpEntry.pos.x);
		data.realRect[1] = static_cast<float>(fpEntry.pos.y);
		data.realRect[2] = static_cast<float>(fpEntry.size.x);
		data.realRect[3] = static_cast<float>(fpEntry.size.y);
		data.useTexture = 1;

		m_compositeCommandBuffer.UpdateUniformBuffer(globalRes.ubo_rect, 0, sizeof(RectDrawData), &data);
		m_compositeCommandBuffer.BindTexture(0, fpEntry.texture);
		m_compositeCommandBuffer.DrawGpuObject(ctxRes.rectObject);
	}

	m_compositeCommandBuffer.EndRenderPass();
	m_compositeCommandBuffer.BlitToScreen(m_framebuffer);
}

SableUI::FloatingPanelEntry& SableUI::Window::GetFloatingPanelEntry(int id)
{
	return m_floatingPanels.at(id);
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
	AsyncTextureLoader::GetInstance().ProcessCompletedLoads(m_mainCommandBuffer);

	if (m_needsRefresh)
	{
		RecalculateNodes();
		RerenderAllNodes();
		m_needsRefresh = false;
	}

	ctx.firedTimers = firedTimers;

	DrawableDrawData drData = DrawableDrawData(
		m_mainCommandBuffer,
		m_framebuffer,
		{ 0, 0, m_windowSize.x, m_windowSize.y },
		GetContextResources(m_renderer)
	);

	ctx.obscurers.clear();
	for (auto& panel : m_floatingPanels)
	{
		Obscurer o{};
		o.r.x = panel.second.pos.x;
		o.r.y = panel.second.pos.y;
		o.r.w = panel.second.size.w;
		o.r.h = panel.second.size.h;
		o.z = panel.second.zIndex;
		ctx.obscurers.push_back(o);
	}

	m_root->DistributeEvents(ctx, 0);

	bool dirty = m_root->UpdateComponents(drData);
	if (dirty)
	{
		m_root->Render(drData);
		m_needsStaticRedraw = true;
	}

	m_root->PostLayoutUpdate(ctx, 0);

	if (!m_resizing)
	{
		StepCachedTexturesCleaner(m_mainCommandBuffer);
		m_renderer->m_textCacheFactory.CleanCache(m_mainCommandBuffer);
	}

	HandleResize();

	if (m_needsRefresh)
	{
		m_mainCommandBuffer.Reset();
		m_mainCommandBuffer.BeginRenderPass(m_framebuffer);
		RecalculateNodes();
		RerenderAllNodes();
		m_needsRefresh = false;
	}

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
	if (IsMinimized()) return;
	SetContext(this);
	MakeContextCurrent();

	if (m_needsStaticRedraw)
	{
		m_mainCommandBuffer.EndRenderPass();
		m_renderer->ExecuteCommandBuffer(m_mainCommandBuffer);
		m_mainCommandBuffer.Reset();
		m_mainCommandBuffer.BeginRenderPass(m_framebuffer);

		for (auto& [id, entry] : m_floatingPanels)
		{
			if (!entry.panel->IsOpen() || !entry.dirty) continue;

			m_renderer->ExecuteCommandBuffer(entry.cmd);
			entry.cmd.Reset();
			entry.cmd.BeginRenderPass(entry.framebuffer);
			entry.dirty = false;
		}

		BuildCompositeCommandBuffer();
		m_renderer->ExecuteCommandBuffer(m_compositeCommandBuffer);
		m_compositeCommandBuffer.Reset();

		m_renderer->CheckErrors();
		glfwSwapBuffers(m_window);

		m_needsStaticRedraw = false;
		m_syncFrames--;
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
	m_renderer->ExecuteCommandBuffer(m_mainCommandBuffer);
	m_mainCommandBuffer.BeginRenderPass(m_framebuffer);

	DrawableDrawData drData = DrawableDrawData(
		m_mainCommandBuffer,
		m_framebuffer,
		{ 0, 0, m_windowSize.x, m_windowSize.y },
		GetContextResources(m_renderer)
	);

	m_root->Render(drData);
	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	DrawableDrawData drData = DrawableDrawData(
		m_mainCommandBuffer,
		m_framebuffer,
		{ 0, 0, m_windowSize.x, m_windowSize.y },
		GetContextResources(m_renderer)
	);

	m_root->Recalculate(drData);
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

// ============================================================================
// Node calculations
// ============================================================================
static void FixWidth(SableUI::BasePanel* panel, const SableUI::DrawableDrawData& drData)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::Base)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixWidth(child, drData);

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

		panel->CalculateScales();
		panel->CalculatePositions(drData);
	}
}

static void FixHeight(SableUI::BasePanel* panel, const SableUI::DrawableDrawData& drData)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::Base)
		return;

	for (SableUI::BasePanel* child : panel->children)
		FixHeight(child, drData);

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

		panel->CalculateScales();
		panel->CalculatePositions(drData);
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

	DrawableDrawData drData = DrawableDrawData(
		m_mainCommandBuffer,
		m_framebuffer,
		{ 0, 0, m_windowSize.x, m_windowSize.y },
		GetContextResources(m_renderer)
	);

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

		FixWidth(state.selectedPanel, drData);
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

		FixHeight(state.selectedPanel, drData);
		break;
	}
	default:
		break;
	}

	state.selectedPanel->parent->CalculateScales();
}

void SableUI::Window::Resize(SableUI::ivec2 pos, SableUI::BasePanel* panel)
{
	static SableUI::ivec2 oldPos = { 0, 0 };
	auto& state = m_resizeState;

	if (panel != nullptr)
	{
		state.selectedPanel = panel;
		state.oldPanelRect = panel->rect;
		state.totalDelta = { 0, 0 };
		state.olderSiblingNode = nullptr;

		if (panel->parent == nullptr) { oldPos = pos; return; }

		auto& siblings = panel->parent->children;
		auto it = std::find(siblings.begin(), siblings.end(), panel);
		if (it == siblings.end() || std::next(it) == siblings.end()) { oldPos = pos; return; }

		state.olderSiblingNode = *std::next(it);
		state.olderSiblingOldRect = state.olderSiblingNode->rect;

		switch (panel->parent->type)
		{
		case PanelType::HorizontalSplitter: state.currentEdgeType = EdgeType::EW_EDGE; break;
		case PanelType::VerticalSplitter:   state.currentEdgeType = EdgeType::NS_EDGE; break;
		default: state.olderSiblingNode = nullptr; break;
		}

		oldPos = pos;
		return;
	}

	if (!state.selectedPanel || !state.olderSiblingNode) return;

	const ivec2 delta = pos - oldPos;
	oldPos = pos;
	if (delta.x == 0 && delta.y == 0) return;

	state.totalDelta = state.totalDelta + delta;

	switch (state.currentEdgeType)
	{
	case EdgeType::EW_EDGE:
	{
		int w = state.oldPanelRect.w + state.totalDelta.x;
		w = (std::max)(w, state.selectedPanel->minBounds.x);
		w = (std::min)(w, state.selectedPanel->parent->rect.w - state.olderSiblingNode->minBounds.x);
		if (state.selectedPanel->maxBounds.x > 0)
			w = (std::min)(w, state.selectedPanel->maxBounds.x);

		state.selectedPanel->wType = RectType::Fixed;
		state.selectedPanel->rect.w = w;
		state.olderSiblingNode->wType = RectType::Fill;
		break;
	}
	case EdgeType::NS_EDGE:
	{
		int h = state.oldPanelRect.h + state.totalDelta.y;
		h = (std::max)(h, state.selectedPanel->minBounds.y);
		h = (std::min)(h, state.selectedPanel->parent->rect.h - state.olderSiblingNode->minBounds.y);
		if (state.selectedPanel->maxBounds.y > 0)
			h = (std::min)(h, state.selectedPanel->maxBounds.y);

		state.selectedPanel->hType = RectType::Fixed;
		state.selectedPanel->rect.h = h;
		state.olderSiblingNode->hType = RectType::Fill;
		break;
	}
	default: return;
	}

	m_needsRefresh = true;
}

SableUI::Window::~Window()
{
	if (m_window)
		MakeContextCurrent();

	SB_delete(m_root);
	DestroyContextResources(m_mainCommandBuffer, m_renderer);

	SB_delete(m_renderer);

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
