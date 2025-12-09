#include <SableUI/window.h>

#include <algorithm>

#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")
#include <windows.h>
#include <dwmapi.h>
#endif

#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <string>
#include <vector>
#include <SableUI/renderer.h>
#include <SableUI/memory.h>
#include <SableUI/textCache.h>
#include <SableUI/console.h>
#include <SableUI/drawable.h>
#include <SableUI/events.h>
#include <SableUI/panel.h>
#include <SableUI/texture.h>
#include <SableUI/utils.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <GL/glew.h>

using namespace SableMemory;

static void SetGLFWContext(GLFWwindow* window, SableUI::Window* sableWindow)
{
	static GLFWwindow* prevContext = nullptr;

	if (prevContext != window)
	{
		glfwMakeContextCurrent(window);
		SableUI::SetCurrentContext(sableWindow);
	}
}

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
	glfwMakeContextCurrent(window);
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	SetGLFWContext(window, instance);
	instance->m_windowSize = ivec2(width, height);
	instance->m_renderer->Viewport(0, 0, width, height);

	instance->m_colourAttachment.CreateStorage(width, height, TextureFormat::RGBA8, TextureUsage::RenderTarget);
	instance->m_framebuffer.SetSize(width, height);
	instance->m_windowSurface.SetSize(width, height);
	instance->m_root->Resize(width, height);
	instance->RecalculateNodes();
	instance->RerenderAllNodes();
	instance->m_needsStaticRedraw = true;
}

void SableUI::Window::WindowRefreshCallback(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}
	SetGLFWContext(window, instance);

	instance->m_needsRefresh = true;
}

void SableUI::Window::ScrollCallback(GLFWwindow* window, double x, double y)
{
	glfwMakeContextCurrent(window);
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}
	SetGLFWContext(window, instance);

	instance->ctx.scrollDelta = { static_cast<int>(x), static_cast<int>(y) };
}

void SableUI::Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_REPEAT) return;
	if (0 > key || key > SABLE_MAX_KEYS) return;

	glfwMakeContextCurrent(window);
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}
	SetGLFWContext(window, instance);

	if (action == GLFW_PRESS)
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

// ============================================================================
// Window
// ============================================================================
SableUI::Window::Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, int x, int y)
{
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	m_windowSize = ivec2(width, height);

	if (primary == nullptr)
	{
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	}
	else
	{
		// Share resources
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, primary->m_window);
	}

	SetGLFWContext(m_window, this);
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

	m_renderer->Flush();

	m_colourAttachment.CreateStorage(m_windowSize.x, m_windowSize.y, TextureFormat::RGBA8, TextureUsage::RenderTarget);
	m_framebuffer.SetSize(m_windowSize.x, m_windowSize.y);
	m_framebuffer.AttachColour(&m_colourAttachment, 0);
	m_framebuffer.Bake();

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

	m_root = SB_new<SableUI::RootPanel>(m_renderer, width, height);
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

bool SableUI::Window::PollEvents()
{
	SetGLFWContext(m_window, this);
	glfwPollEvents();

	if (m_needsRefresh)
	{
		m_renderer->ClearDrawableStack();
		RecalculateNodes();
		RerenderAllNodes();
		m_needsRefresh = false;
	}

	m_root->PropagateEvents(ctx);
	m_root->PropagateComponentStateChanges();

	StepCachedTexturesCleaner();
	TextCacheFactory::CleanCache(m_renderer);

	HandleResize();

	ctx.mousePressed.reset();
	ctx.mouseReleased.reset();
	ctx.mouseDoubleClicked.reset();
	ctx.scrollDelta = { 0, 0 };

	return !glfwWindowShouldClose(m_window);
}

void SableUI::Window::Draw()
{
	SetGLFWContext(m_window, this);

#ifdef _WIN32
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			exit(0);
		}
	}
#endif

	bool blitted = false;
	bool wasDirty = false;
	bool needsFlush = false;

	// if layout is dirty rerender to windows custom framebuffer
	if (m_renderer->isDirty())
	{
		wasDirty = true;
		m_renderer->BeginRenderPass(&m_framebuffer);
		bool res = m_renderer->Draw(&m_framebuffer);
		m_renderer->EndRenderPass();

		needsFlush = needsFlush || res;
	}

	if (m_customTargetQueues.size() != 0)
	{
		// track if we need to blit custom targets to window surface
		bool needsToBlitDefault = false;
		for (CustomTargetQueue* queue : m_customTargetQueues)
			needsToBlitDefault = needsToBlitDefault || queue->target == &m_windowSurface;

		// if we do, blit old non-dirty custom fbo to window surface
		if (needsToBlitDefault)
		{
			m_renderer->BlitToScreen(&m_framebuffer);
			blitted = true;
		}
	}

	// execute custom queues
	for (CustomTargetQueue* queue : m_customTargetQueues)
	{
		bool res = false;
		if (queue->drawables.size() != 0)
			res = true;

		for (DrawableBase* dr : queue->drawables)
			m_renderer->AddToDrawStack(dr);

		if (queue->root)
		{
			res = true;
			queue->root->LayoutChildren();
			queue->root->Render();
		}

		m_renderer->BeginRenderPass(queue->target);
		if (m_renderer->Draw(queue->target));
			res = true;

		m_renderer->EndRenderPass();

		needsFlush = needsFlush || res;
	}

	if ((wasDirty && !blitted))
		m_renderer->BlitToScreen(&m_framebuffer);

	if (needsFlush) // has surface changed? flush changes
	{
		m_renderer->CheckErrors();
		m_renderer->Flush();
	}
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
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::BASE) {
		return;
	}

	for (SableUI::BasePanel* child : panel->children)
	{
		FixWidth(child);
	}

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.w < child->minBounds.x)
		{
			child->rect.w = child->minBounds.x;
			child->rect.wType = SableUI::RectType::FIXED;
			resized = true;
		}

		if (child->maxBounds.x > 0 && child->rect.w > child->maxBounds.x)
		{
			int excess = child->rect.w - child->maxBounds.x;
			child->rect.w = child->maxBounds.x;
			child->rect.wType = SableUI::RectType::FIXED;
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

			if (child->rect.wType == SableUI::RectType::FIXED ||
				child->rect.wType == SableUI::RectType::FILL)
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
					panels[i]->rect.wType = SableUI::RectType::FIXED;

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
			if (child->rect.wType == SableUI::RectType::FIXED)
			{
				fixedWidthSum += child->rect.w;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->rect.wType != SableUI::RectType::FIXED)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.w = fixedWidthSum;
			panel->rect.wType = SableUI::RectType::FIXED;
		}

		panel->CalculateScales();
		panel->CalculatePositions();
	}
}

static void FixHeight(SableUI::BasePanel* panel)
{
	if (panel->children.size() == 0 || panel->type == SableUI::PanelType::BASE) return;

	for (SableUI::BasePanel* child : panel->children)
	{
		FixHeight(child);
	}

	bool resized = false;
	int deficit = 0;

	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.h < child->minBounds.y)
		{
			child->rect.h = child->minBounds.y;
			child->rect.hType = SableUI::RectType::FIXED;
			resized = true;
		}

		if (child->maxBounds.y > 0 && child->rect.h > child->maxBounds.y)
		{
			int excess = child->rect.h - child->maxBounds.y;
			child->rect.h = child->maxBounds.y;
			child->rect.hType = SableUI::RectType::FIXED;
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

			if (child->rect.hType == SableUI::RectType::FIXED ||
				child->rect.hType == SableUI::RectType::FILL)
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
					panels[i]->rect.hType = SableUI::RectType::FIXED;

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
			if (child->rect.hType == SableUI::RectType::FIXED)
			{
				fixedHeightSum += child->rect.h;
			}
		}

		bool allChildrenFixed = true;
		for (SableUI::BasePanel* child : panel->children)
		{
			if (child->rect.hType != SableUI::RectType::FIXED)
			{
				allChildrenFixed = false;
				break;
			}
		}

		if (allChildrenFixed)
		{
			panel->rect.h = fixedHeightSum;
			panel->rect.hType = SableUI::RectType::FIXED;
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

		state.selectedPanel->rect.wType = SableUI::RectType::FIXED;
		state.selectedPanel->rect.w = width;

		state.olderSiblingNode->rect.wType = SableUI::RectType::FILL;

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

		state.selectedPanel->rect.hType = SableUI::RectType::FIXED;
		state.selectedPanel->rect.h = height;

		state.olderSiblingNode->rect.hType = SableUI::RectType::FILL;

		FixHeight(state.selectedPanel);
		break;
	}
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
	glfwMakeContextCurrent(m_window);
	SB_delete(m_root);
	DestroyDrawables();
	TextCacheFactory::ShutdownFactory(m_renderer);

	SB_delete(m_renderer);
	glfwDestroyWindow(m_window);
}

void SableUI::SableUI_Window_Initalise_GLFW()
{
	if (!glfwInit())
	{
		SableUI_Runtime_Error("Could not initialize GLFW");
	}
}

void SableUI::SableUI_Window_Terminate_GLFW()
{
	glfwTerminate();
}
