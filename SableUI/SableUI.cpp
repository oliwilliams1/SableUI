#include "SableUI/SableUI.h"
#include "SableUI/shader.h"
#include "SableUI/text.h"

#include <cstdio>
#include <iostream>
#include <functional>
#include <algorithm>
#include <stack>
#include <chrono>
#include <thread>
#include <tinyxml2.h>

#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")
#include <windows.h>
#include <dwmapi.h>
#endif

static float DistToEdge(SableUI::Node* node, SableUI::ivec2 p)
{
	SableUI::Rect r = node->rect;

	SableUI::NodeType parentType = (node->parent == nullptr) ? SableUI::NodeType::ROOTNODE : node->parent->type;

	switch (parentType)
	{
		case SableUI::NodeType::ROOTNODE:
		{
			float distLeft = p.x - r.x;
			float distRight = (r.x + r.w) - p.x;
			float distTop = p.y - r.y;
			float distBottom = (r.y + r.h) - p.y;

			return (std::max)(0.0f, (std::min)({ distLeft, distRight, distTop, distBottom }));
		}

		case SableUI::NodeType::HSPLITTER:
		{
			float distRight = (r.x + r.w) - p.x;
			return (distRight < 0) ? 0 : distRight;
		}

		case SableUI::NodeType::VSPLITTER:
		{
			float distBottom = (r.y + r.h) - p.y;
			return (distBottom < 0) ? 0 : distBottom;
		}

		default:
			return 0.0f;
	}
}

SableUI::Window::Window(const std::string& title, int width, int height, int x, int y)
{
	if (!m_initialized)
	{
		if (!glfwInit())
		{
			SableUI_Runtime_Error("Could not initialize GLFW");
		}
	}

	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	m_windowSize = ivec2(width, height);
	m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));

#ifdef _WIN32
	/* enable immersive darkmode on windows via api */
	HWND hwnd = FindWindowA(NULL, title.c_str());

	BOOL dark_mode = true;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));

	// Force update of immersive dark mode
	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);
#endif

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glFlush();

	// init after window is cleared
	GLenum res = glewInit();
	if (GLEW_OK != res)
	{
		SableUI_Runtime_Error("Could not initialize GLEW: %s", glewGetErrorString(res));
	}

	int refreshRate = GetRefreshRate();
	if (refreshRate == -1) refreshRate = 60;
	/* set internal max hz to display refresh rate */
	SetMaxFPS(refreshRate);
	
	InitFontManager();
	InitDrawables();

	m_renderer.renderTarget.SetTarget(TargetType::WINDOW);
	m_renderer.renderTarget.Resize(m_windowSize.x, m_windowSize.y);

	if (m_root != nullptr)
	{
		SableUI_Error("Root node already created!");
		return;
	}

	m_arrowCursor   = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_hResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_vResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

	glfwSetCursorPosCallback(m_window, MousePosCallback);
	glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
	glfwSetWindowSizeCallback(m_window, ResizeCallback);

	/* make root node */
	m_root = new SableUI::RootNode(&m_renderer, width, height);
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

	instance->m_mousePos = { static_cast<int>(x), static_cast<int>(y) };
}

void SableUI::Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Window* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!instance)
	{
		SableUI_Runtime_Error("Could not get window instance");
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		instance->m_mouseButtonStates.LMB = MouseState::DOWN;
	}
	else if (action == GLFW_RELEASE)
	{
		instance->m_mouseButtonStates.LMB = MouseState::UP;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		instance->m_mouseButtonStates.RMB = MouseState::DOWN;
	} 
	else if (action == GLFW_RELEASE)
	{
		instance->m_mouseButtonStates.RMB = MouseState::UP;
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
	glViewport(0, 0, width, height);

	instance->m_renderer.renderTarget.Resize(width, height);

	instance->m_root->Resize(width, height);
	instance->RecalculateNodes();
	instance->RerenderAllNodes();

	instance->m_needsStaticRedraw = true;
}

GLFWcursor* SableUI::Window::CheckResize(Node* node, bool* resCalled)
{
	if (node == nullptr) return m_arrowCursor;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (RectBoundingBox(node->rect, m_mousePos) && m_mousePos.x != 0 && m_mousePos.y != 0)
	{
		float d1 = DistToEdge(node, m_mousePos);

		if (node->parent != nullptr && d1 < 5.0f)
		{
			switch (node->parent->type)
			{
			case NodeType::VSPLITTER:
				cursorToSet = m_vResizeCursor;
				break;

			case NodeType::HSPLITTER:
				cursorToSet = m_hResizeCursor;
				break;
			}

			if (!m_resizing && m_mouseButtonStates.LMB == MouseState::DOWN && cursorToSet != m_arrowCursor)
			{
				*resCalled = true;
				Resize(m_mousePos, node);
				m_resizing = true;
			}
		}
	}

	for (Node* child : node->children)
	{
		GLFWcursor* childCursor = CheckResize(child, resCalled);
		if (childCursor != m_arrowCursor)
		{
			cursorToSet = childCursor;
		}
	}

	return cursorToSet;
}

bool SableUI::Window::PollEvents()
{
	static bool init = false;
	glfwPollEvents();
	
	if (!init)
	{
		m_renderer.Flush(); // Clear the draw stack from init draw commands
		RecalculateNodes(); // Recalc everything after init
		RerenderAllNodes();
		Draw();             // Redraw from fresh stack
		init = true;
	}

	// static for multiple calls on one resize event (lifetime of static is until mouse up)
	static bool resCalled = false;

	GLFWcursor* cursorToSet = m_arrowCursor;

	if (DistToEdge(m_root, m_mousePos) > 5.0f)
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
			Resize(m_mousePos);
		}
		else
		{
			resCalled = false;
		}
		if (m_mouseButtonStates.LMB == MouseState::UP)
		{
			m_resizing = false;
			m_renderer.Flush();
			RecalculateNodes();
			RerenderAllNodes();
		}
	}

	return !glfwWindowShouldClose(m_window);
}

void SableUI::Window::Draw()
{
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

//static int elementCtr = 0;
//SableUI::Element* SableUI::Window::AddElementToElement(const std::string& elementName, ElementInfo& p_info, ElementType type)
//{
//	ElementInfo info = p_info;
//	if (type == ElementType::TEXT && info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;
//	if (type != ElementType::TEXT && info.hType == RectType::UNDEF) info.hType = RectType::FILL;
//	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
//
//	if (info.name.length() == 0)
//	{
//		info.name = std::to_string(elementCtr++);
//	}
//
//	Element* parent = m_renderer.GetElement(elementName);
//
//	if (parent->type == ElementType::IMAGE || parent->type == ElementType::TEXT)
//	{
//		SableUI_Warn("Cannot add element to image or text element");
//		return nullptr;
//	}
//
//	if (parent == nullptr)
//	{
//		SableUI_Warn("Cannot find element: %s!", elementName.c_str());
//		return nullptr;
//	}
//
//	Element* child = m_renderer.CreateElement(info.name, type);
//	if (child == nullptr)
//	{
//		SableUI_Error("Failed to create element: %s", info.name.c_str());
//		return nullptr;
//	}
//
//	info.type = type;
//	child->SetInfo(info);
//	parent->AddChild(child);
//
//	RecalculateNodes();
//
//	return child;
//}

SableUI::RootNode* SableUI::Window::GetRoot()
{
	return m_root;
}

void SableUI::Window::RerenderAllNodes()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	m_root->Render();

	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	m_root->Recalculate();
}

void SableUI::Window::Resize(SableUI::ivec2 pos, SableUI::Node* node)
{
	m_needsStaticRedraw = true;

	static SableUI::Node* selectedNode = nullptr;
	static SableUI::EdgeType currentEdgeType = SableUI::EdgeType::NONE;
	static SableUI::ivec2 oldPos = { 0, 0 };
	static SableUI::Rect oldNodeRect = { 0, 0, 0, 0 };
	static SableUI::Node* olderSiblingNode = nullptr;
	static SableUI::Rect olderSiblingOldRect = { 0, 0, 0, 0 };

	if (node != nullptr)
	{
		oldPos = pos;
		selectedNode = node;
		oldNodeRect = node->rect;

		if (node->parent == nullptr)
		{
			olderSiblingNode = nullptr;
			olderSiblingOldRect = { 0, 0, 0, 0 };
			return;
		}

		auto& siblings = node->parent->children;
		auto it = std::find(siblings.begin(), siblings.end(), selectedNode);
		if (it != siblings.end() && std::next(it) != siblings.end())
		{
			olderSiblingNode = *std::next(it);
			olderSiblingOldRect = olderSiblingNode->rect;
		}
		else
		{
			olderSiblingNode = nullptr;
			olderSiblingOldRect = { 0, 0, 0, 0 };
			return;
		}

		switch (node->parent->type)
		{
		case SableUI::NodeType::HSPLITTER:
			currentEdgeType = SableUI::EdgeType::EW_EDGE;
			break;
		case SableUI::NodeType::VSPLITTER:
			currentEdgeType = SableUI::EdgeType::NS_EDGE;
			break;
		default:
			return;
		}

		return;
	}

	SableUI::ivec2 deltaPos = pos - oldPos;

	static SableUI::ivec2 prevPos = { 0, 0 };
	SableUI::ivec2 dPos = prevPos - pos;
	if (dPos.x == 0 && dPos.y == 0) return;
	prevPos = pos;

	if (currentEdgeType == SableUI::EdgeType::EW_EDGE)
	{
		int minWidth = CalculateMinimumWidth(node);
		deltaPos.x = std::clamp(deltaPos.x,
			-oldNodeRect.w + minWidth,
			olderSiblingOldRect.w - minWidth);

		int newWidth = (std::max)(oldNodeRect.w + deltaPos.x, minWidth);
		selectedNode->rect.w = newWidth;
		olderSiblingNode->rect.w = olderSiblingOldRect.w - (newWidth - oldNodeRect.w);

		selectedNode->rect.wType = SableUI::RectType::FIXED;

		for (SableUI::Node* sibling : selectedNode->parent->children)
		{
			if (sibling != selectedNode)
			{
				sibling->rect.wType = SableUI::RectType::FILL;
				sibling->rect.hType = SableUI::RectType::FILL;
			}
		}

		selectedNode->parent->CalculateScales();
	}
	else if (currentEdgeType == SableUI::EdgeType::NS_EDGE)
	{
		int minHeight = CalculateMinimumHeight(node);
		deltaPos.y = std::clamp(deltaPos.y,
			-oldNodeRect.h + minHeight,
			olderSiblingOldRect.h - minHeight);

		selectedNode->rect.h = oldNodeRect.h + deltaPos.y;
		olderSiblingNode->rect.h = olderSiblingOldRect.h - deltaPos.y;

		selectedNode->rect.hType = SableUI::RectType::FIXED;

		for (SableUI::Node* sibling : selectedNode->parent->children)
		{
			if (sibling != selectedNode)
			{
				sibling->rect.hType = SableUI::RectType::FILL;
				sibling->rect.wType = SableUI::RectType::FILL;
			}
		}

		selectedNode->parent->CalculateScales();
	}

	m_root->Recalculate();
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


int SableUI::Window::CalculateMinimumWidth(Node* node)
{
	/*if (node == nullptr) return 20;

	if (node->type == NodeType::BASE)
	{
		if (auto* defaultComponent = dynamic_cast<Body*>(node->m_component))
		{
			return defaultComponent->m_element->GetWidth();
		}
		return 20;
	}
	else if (node->type == NodeType::HSPLITTER)
	{
		int totalMinChildWidth = 0;
		for (SableUI::Node* child : node->children)
		{
			totalMinChildWidth += CalculateMinimumWidth(child);
		}
		return (std::max)(totalMinChildWidth, 2 * node->bSize);
	}
	else if (node->type == NodeType::VSPLITTER)
	{
		int maxChildWidth = 0;
		for (SableUI::Node* child : node->children)
		{
			maxChildWidth = (std::max)(maxChildWidth, CalculateMinimumWidth(child));
		}
		return (std::max)(maxChildWidth, 20);
	}*/

	return 20;
}

int SableUI::Window::CalculateMinimumHeight(Node* node)
{
	/*if (node == nullptr) return 20;

	if (node->type == NodeType::BASE)
	{
		if (auto* defaultComponent = dynamic_cast<Body*>(node->m_component))
		{
			return defaultComponent->m_element->GetHeight();
		}
		return 20; 
	}
	else if (node->type == NodeType::VSPLITTER)
	{
		int totalMinChildHeight = 0;
		for (SableUI::Node* child : node->children)
		{
			totalMinChildHeight += CalculateMinimumHeight(child);
		}
		return (std::max)(totalMinChildHeight, 2 * node->bSize);
	}
	else if (node->type == NodeType::HSPLITTER)
	{
		int maxChildHeight = 0;
		for (SableUI::Node* child : node->children)
		{
			maxChildHeight = (std::max)(maxChildHeight, CalculateMinimumHeight(child));
		}
		return (std::max)(maxChildHeight, 20);
	}*/

	return 20;
}

void SableUI::Window::CalculateAllNodeMinimumBounds()
{
	/*for (Node* node : m_nodes)
	{
		int bSize = 0;
		if (node->parent) bSize = node->parent->bSize;
		node->minBounds = { CalculateMinimumWidth(node) + 2 * bSize, CalculateMinimumHeight(node) + 2 * bSize};
	}*/
}

SableUI::Window::~Window()
{
	delete m_root;

	DestroyFontManager();
	DestroyDrawables();
	DestroyShaders();

	glfwDestroyWindow(m_window);
	glfwTerminate();

	SableUI_Log("Shut down successfully");
}