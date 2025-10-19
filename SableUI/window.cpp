#include "SableUI/window.h"
#include "SableUI/memory.h"

#include <cstdio>
#include <iostream>
#include <functional>
#include <algorithm>
#include <stack>
#include <chrono>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

void* SableUI::GetCurrentContext()
{
	return static_cast<void*>(glfwGetCurrentContext());
}

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

	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));

#ifdef _WIN32
	// Enable immersive darkmode on windows via api 
	HWND hwnd = FindWindowA(NULL, title.c_str());

	BOOL dark_mode = true;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));

	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);
#endif

	int refreshRate = GetRefreshRate();
	if (refreshRate == -1) refreshRate = 60;
	// Set internal max hz to display refresh rate
	SetMaxFPS(refreshRate);

	if (primary == nullptr)
	{
		switch (backend)
		{
		case Backend::OpenGL:
		{
			InitOpenGL();
			break;
		}
		case Backend::Vulkan:
		{
			SableUI_Runtime_Error("Vulkan backend is not implemented");
			break;
		}
		default:
			SableUI_Runtime_Error("unknown backend");
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glFlush();

	m_renderer.renderTarget.SetTarget(TargetType::WINDOW);
	m_renderer.renderTarget.Resize(m_windowSize.x, m_windowSize.y);

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

	m_root = SB_new<SableUI::RootPanel>(&m_renderer, width, height);
}

void SableUI::Window::InitOpenGL()
{
	SableUI_Log("Using OpenGL backend");

	// init after window is cleared
	GLenum res = glewInit();
	if (GLEW_OK != res)
	{
		SableUI_Runtime_Error("Could not initialize GLEW: %s", glewGetErrorString(res));
	}
}

/* callbacks */
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

	instance->m_windowSize = ivec2(width, height);
	glViewport(0, 0, width, height);

	instance->m_renderer.renderTarget.Resize(width, height);
	instance->m_root->Resize(width, height);
	instance->RecalculateNodes();
	instance->RerenderAllNodes();
	instance->m_needsStaticRedraw = true;
}

void SableUI::Window::HandleResize()
{
	// static for multiple calls on one resize event (lifetime of static is until mouse up)
	static bool resCalled = false;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (DistToEdge(m_root, ctx.mousePos) > 5.0f)
	{
		cursorToSet = CheckResize(m_root, &resCalled);
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
			m_renderer.ClearStack();
			RecalculateNodes();
			RerenderAllNodes();
		}
	}
}

GLFWcursor* SableUI::Window::CheckResize(BasePanel* node, bool* resCalled)
{
	if (node == nullptr) return m_arrowCursor;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (RectBoundingBox(node->rect, ctx.mousePos) && ctx.mousePos.x != 0 && ctx.mousePos.y != 0)
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
		GLFWcursor* childCursor = CheckResize(child, resCalled);
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
	glfwMakeContextCurrent(m_window);
	glfwPollEvents();

	m_root->PropagateEvents(ctx);
	m_root->PropagateComponentStateChanges();
	StepCachedTexturesCleaner();

	HandleResize();

	ctx.mousePressed.reset();
	ctx.mouseReleased.reset();

	return !glfwWindowShouldClose(m_window);
}

void SableUI::Window::Draw()
{
	glfwMakeContextCurrent(m_window);
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		SableUI_Error("OpenGL error: %s", gluErrorString(err));
	}
	auto frameStart = std::chrono::system_clock::now();

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

	/* flush drawable stack & render to screen */
	m_renderer.Draw();

	/* ensure application doesnt run faster than desired fps */
	auto frameTime = std::chrono::system_clock::now() - frameStart;

	if (frameTime < m_frameDelay)
	{
		std::this_thread::sleep_for(m_frameDelay - frameTime);
	}
}

inline void SableUI::Window::SetMaxFPS(int fps)
{
	m_frameDelay = std::chrono::milliseconds(1000 / fps);
}

SableUI::RootPanel* SableUI::Window::GetRoot()
{
	return m_root;
}

void SableUI::Window::RerenderAllNodes()
{
	m_renderer.ClearStack();

	glClear(GL_COLOR_BUFFER_BIT);

	m_root->Render();

	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	m_root->Recalculate();
}

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
	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.w < child->minBounds.x)
		{
			child->rect.w = child->minBounds.x;
			child->rect.wType = SableUI::RectType::FIXED;
			resized = true;
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
	for (SableUI::BasePanel* child : panel->children)
	{
		if (child->rect.h < child->minBounds.y)
		{
			child->rect.h = child->minBounds.y;
			child->rect.hType = SableUI::RectType::FIXED;
			resized = true;
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
	m_renderer.ClearStack();
	m_root->Recalculate();

	oldPos = pos;
}

int SableUI::Window::GetRefreshRate()
{
	GLFWmonitor* monitor = glfwGetWindowMonitor(m_window);
	if (!monitor)
	{
		int xpos, ypos;
		glfwGetWindowPos(m_window, &xpos, &ypos);
		monitor = glfwGetPrimaryMonitor();
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		for (int i = 0; i < monitorCount; i++)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			if (xpos >= mode->width && xpos < mode->width + mode->width &&
				ypos >= mode->height && ypos < mode->height + mode->height)
			{
				monitor = monitors[i];
				break;
			}
		}

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		int widthMM, heightMM;
		glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

		windowDPI.x = (mode->width / static_cast<float>(widthMM)) * 25.4f;
		windowDPI.y = (mode->height / static_cast<float>(heightMM)) * 25.4f;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (mode)
	{
		return mode->refreshRate;
	}
	else
	{
		return -1;
	}
}

SableUI::Window::~Window()
{
	glfwMakeContextCurrent(m_window);

	SB_delete(m_root);
	DestroyDrawables();
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
