#include "SableUI/SableUI.h"
#include "SableUI/shader.h"

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

constexpr float minComponentSize = 20.0f;

static float DistToEdge(SableUI::Node* node, SableUI::ivec2 p)
{
	SableUI::Rect r = node->rect;
	SableUI::NodeType parentType = SableUI::NodeType::ROOTNODE;

	if (node->parent != nullptr)
	{
		parentType = node->parent->type;
	}

	if (parentType == SableUI::NodeType::HSPLITTER) {
		float distRight = (r.x + r.w) - p.x;
		return (distRight < 0) ? 0 : distRight;
	}

	if (parentType == SableUI::NodeType::VSPLITTER)
	{
		float distBottom = (r.y + r.h) - p.y;
		return (distBottom < 0) ? 0 : distBottom;
	}

	return 0;
}

static void PrintNode(SableUI::Node* node, int depth = 0)
{
	if (node == nullptr) return;

	std::string indent(depth * 2, ' ');

	size_t nameLength = node->name.length();
	int spacesNeeded = 24 - (int)nameLength;

	if (spacesNeeded < 0) spacesNeeded = 0;

	SableUI_Log("%s name: %s, w: %.2f, h: %.2f, x: %.2f, y: %.2f, htype: %i, wtype: %i",
		indent.c_str(), node->name.c_str(), node->rect.w,
		node->rect.h, node->rect.x, node->rect.y,
		node->rect.hType, node->rect.wType);

	for (SableUI::Node* child : node->children)
	{
		PrintNode(child, depth + 1);
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

	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	m_windowSize = ivec2(width, height);
	m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(m_window);

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

	/* set internal max hz to display refresh rate */
	SetMaxFPS(60);
	
	InitFontManager();
	InitDrawables();

	renderer.renderTarget.SetTarget(TargetType::WINDOW);
	renderer.renderTarget.Resize(m_windowSize.x, m_windowSize.y);

	if (m_root != nullptr)
	{
		SableUI_Error("Root node already created!");
		return;
	}

	m_arrowCursor   = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_hResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_vResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

	/* event callbacks */
	/*glutReshapeFunc(ReshapeCallback);
	glutMotionFunc(MotionCallback);
	glutPassiveMotionFunc(MotionCallback);
	glutMouseFunc(MouseButtonCallback);*/

	/* make root node */
	m_root = new SableUI::Node(NodeType::ROOTNODE, nullptr, "Root Node");
	SetupRootNode(m_root, width, height);
	m_nodes.push_back(m_root);
}

bool SableUI::Window::PollEvents()
{
	static bool init = false;
	glfwPollEvents(); // This is the heart of event handling

	if (!init)
	{
		//PrintNodeTree();
		renderer.Flush();   // Clear the draw stack from init draw commands
		RecalculateNodes(); // Recalc everything after init
		RerenderAllNodes();
		Draw();             // Redraw from fresh stack
		init = true;
	}

	// static for multiple calls on one resize event (lifetime of static is until mouse up)
	static bool resCalled = false;

	GLFWcursor* cursorToSet = m_arrowCursor;

	/* check if need to resize */
	for (Node* node : m_nodes)
	{
		if (DistToEdge(m_root, m_mousePos) > 5.0f) continue;
		if (!RectBoundingBox(node->rect, m_mousePos)) continue;

		if (m_mousePos.x == 0 && m_mousePos.y == 0) continue;

		float d1 = DistToEdge(node, m_mousePos);

		if (node->parent == nullptr) continue;

		if (d1 < 5.0f)
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
				resCalled = true;
				Resize(m_mousePos, node);

				m_resizing = true;
				continue;
			}
		}
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
			renderer.Flush();
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

	if (m_needsStaticRedraw)
	{
		/* flush drawable stack & render to screen */
		renderer.Draw();

		glFlush();

		m_needsStaticRedraw = false;
	}

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

void SableUI::Window::PrintNodeTree()
{
	PrintNode(m_root);
}

void SableUI::Window::SetupSplitter(const std::string& name, float bSize)
{
	Node* node = FindNodeByName(name);

	if (node == nullptr) return;

	node->bSize = bSize;
}

SableUI::Node* SableUI::Window::AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName)
{
	CalculateNodePositions();

	SableUI::Node* parent = FindNodeByName(parentName);

	if (parent == nullptr)
	{
		SableUI_Error("Parent node is null!");
		return nullptr;
	}

	if (parent->type == NodeType::COMPONENT)
	{
		SableUI_Error("Cannot create a child node on a component node");
		return nullptr;
	}

	if (parent->type == NodeType::ROOTNODE && m_root->children.size() > 0)
	{
		SableUI_Error("Root node can only have one child node");
		return nullptr;
	}

	SableUI::Node* node = new SableUI::Node(type, parent, name);

	m_nodes.push_back(node);

	return node;
}

void SableUI::Window::AttachComponentToNode(const std::string& nodeName, std::unique_ptr<SableUI::BaseComponent> component)
{
	Node* node = FindNodeByName(nodeName);

	if (node == nullptr)
	{
		SableUI_Warn("Cannot find node: %s!", nodeName.c_str());
		return;
	}

	if (node->component != nullptr)
	{
		SableUI_Error("Node: %s already has a component attached!", nodeName.c_str());
		return;
	}

	node->component = std::move(component);
	node->component->SetParent(node);
	node->component->SetRenderer(&renderer);
}

SableUI::Element* SableUI::Window::AddElementToComponent(const std::string& nodeName, ElementInfo& info, ElementType type)
{
	if (info.name.length() == 0) { SableUI_Error("Element name cannot be empty!"); return nullptr; }

	Node* node = FindNodeByName(nodeName);

	if (node == nullptr)
	{
		SableUI_Warn("Cannot find node: %s!", nodeName.c_str());
		return nullptr;
	}

	if (node->type == NodeType::COMPONENT)
	{
		if (auto* defaultComponent = dynamic_cast<DefaultComponent*>(node->component.get()))
		{
			Element* element = renderer.CreateElement(info.name, type);
			if (element == nullptr) SableUI_Error("Failed to create element: %s", info.name.c_str());

			info.type = type;
			element->SetInfo(info);
			defaultComponent->AddElement(element);
			RecalculateNodes();
			return element;
		}
	}
	else
	{
		SableUI_Error("Adding element to non-component node: %s", nodeName.c_str());
	}

	return nullptr;
}

SableUI::Element* SableUI::Window::AddElementToElement(const std::string& elementName, ElementInfo& info, ElementType type)
{
	if (info.name.length() == 0) { SableUI_Error("Element name cannot be empty!"); return nullptr; }

	Element* parent = renderer.GetElement(elementName);

	if (parent == nullptr)
	{
		SableUI_Warn("Cannot find element: %s!", elementName.c_str());
		return nullptr;
	}

	Element* child = renderer.CreateElement(info.name, type);
	if (child == nullptr) SableUI_Error("Failed to create element: %s", info.name.c_str());

	info.type = type;
	child->SetInfo(info);
	parent->AddChild(child);

	RecalculateNodes();

	return child;
}

SableUI::Node* SableUI::Window::GetRoot()
{
	return m_root;
}

SableUI::Node* SableUI::Window::FindNodeByName(const std::string& name)
{
	for (SableUI::Node* node : m_nodes)
	{
		if (node->name == name)
		{
			return node;
		}
	}

	return nullptr;
}

void SableUI::Window::OpenUIFile(const std::string& path)
{
	if (m_nodes.size() > 0)
	{
		for (Node* node : m_nodes)
		{
			delete node;
		}
		m_nodes.clear();
		m_root = nullptr;
		SableUI_Log("Reloading UI");
	}

	using namespace tinyxml2;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
	{
		SableUI_Notify_Error("Failed to load file: %s", path.c_str(), true);
		return;
	}

	XMLElement* rootElement = doc.FirstChildElement("root");
	if (!rootElement) {
		SableUI_Error("Invalid XML structure: Missing <root> element");
		return;
	}

	m_root = new Node(NodeType::ROOTNODE, nullptr, "Root Node");

	SetupRootNode(m_root, m_windowSize.x, m_windowSize.y);
	m_nodes.push_back(m_root);

	std::stack<std::string> parentStack;
	parentStack.push("Root Node");

	std::function<void(XMLElement*, const std::string&)> parseNode;
	parseNode = [&](XMLElement* element, const std::string& parentName)
	{
		while (element)
		{
			std::string elementName = element->Name();
			const char* nameAttr = element->Attribute("name");

			const char* colourAttr = element->Attribute("colour");
			SableUI::Colour colour = SableUI::Colour(51, 51, 51);
			if (colourAttr) colour = StringTupleToColour(colourAttr);

			const char* borderAttr = element->Attribute("border");
			float border = 1.0f;
			if (borderAttr) border = std::stof(borderAttr);

			const char* borderColourAttr = element->Attribute("borderColour");
			SableUI::Colour borderColour = StringTupleToColour(borderColourAttr);

			std::string nodeName = (nameAttr) ? nameAttr : elementName + " " + std::to_string(m_nodes.size());

			if (elementName == "hsplitter")
			{
				AddNodeToParent(NodeType::HSPLITTER, nodeName, parentName);
				SetupSplitter(nodeName, border);
				AttachComponentToNode(nodeName, std::make_unique<SplitterComponent>(colour));

				parentStack.push(nodeName);
				parseNode(element->FirstChildElement(), nodeName);
				parentStack.pop();
			}
			else if (elementName == "vsplitter")
			{
				AddNodeToParent(NodeType::VSPLITTER, nodeName, parentName);
				SetupSplitter(nodeName, border);
				AttachComponentToNode(nodeName, std::make_unique<SplitterComponent>(colour));

				parentStack.push(nodeName);
				parseNode(element->FirstChildElement(), nodeName);
				parentStack.pop();
			}
			else if (elementName == "component")
			{
				AddNodeToParent(NodeType::COMPONENT, nodeName, parentName);
				AttachComponentToNode(nodeName, std::make_unique<DefaultComponent>(colour));
			}

			element = element->NextSiblingElement();
		}
	};

	parseNode(rootElement->FirstChildElement(), parentStack.top());
}

void SableUI::Window::RerenderAllNodes()
{
	glClear(GL_COLOR_BUFFER_BIT);
	for (SableUI::Node* node : m_nodes)
	{
		if (node->component)
		{
			node->component.get()->UpdateDrawable();
		}
	}
	m_needsStaticRedraw = true;
	Draw();
}

void SableUI::Window::RecalculateNodes()
{
	CalculateNodeScales();
	CalculateNodePositions();

	for (Node* node : m_nodes)
	{
		if (node->component != nullptr && node->type != NodeType::COMPONENT)
		{
			node->component.get()->UpdateDrawable(false);
		}
	}
}

void SableUI::Window::Resize(SableUI::vec2 pos, SableUI::Node* node)
{
	m_needsStaticRedraw = true;

	/* static for multiple calls (lifetime of static is until mouse up) */
	static SableUI::Node* selectedNode = nullptr;
	static SableUI::EdgeType currentEdgeType = SableUI::EdgeType::NONE;

	static SableUI::vec2 oldPos = { 0, 0 };
	static SableUI::Rect oldNodeRect = { 0, 0, 0, 0 };
	static float nParentsChildren = 1.0f; // prev div 0

	static SableUI::Node* olderSiblingNode = nullptr;
	static SableUI::Rect olderSiblingOldRect = { 0, 0, 0, 0 };

	// First resize call fills data
	if (node != nullptr)
	{
		oldPos = pos;
		selectedNode = node;
		oldNodeRect = node->rect;
		nParentsChildren = static_cast<float>(node->parent->children.size());

		if (node->parent == nullptr)
		{
			olderSiblingNode = nullptr;
			olderSiblingOldRect = { 0, 0, 0, 0 };
			return;
		}

		int olderSiblingIndex = node->index + 1;
		if (olderSiblingIndex < node->parent->children.size())
		{
			olderSiblingNode = node->parent->children[olderSiblingIndex];
			olderSiblingOldRect = olderSiblingNode->rect;
		}
		else return;

		switch (node->parent->type)
		{
		case SableUI::NodeType::HSPLITTER:
			currentEdgeType = SableUI::EdgeType::EW_EDGE;
			break;

		case SableUI::NodeType::VSPLITTER:
			currentEdgeType = SableUI::EdgeType::NS_EDGE;
			break;
		}

		return;
	}

	SableUI::vec2 deltaPos = pos - oldPos;
	
	// Prevent continuing if delta pos since last call is negligible
	{
		static SableUI::vec2 prevPos = { 0, 0 };
		SableUI::vec2 dPos = prevPos - pos;
		if (dPos.x == 0 && dPos.y == 0) return;
		prevPos = pos;
	}

	if (currentEdgeType == SableUI::EW_EDGE)
	{
		deltaPos.x = std::clamp(deltaPos.x,
			-oldNodeRect.w + minComponentSize,
			olderSiblingOldRect.w - minComponentSize);

		selectedNode->rect.w = oldNodeRect.w + deltaPos.x;
		olderSiblingNode->rect.w = olderSiblingOldRect.w - deltaPos.x;

		selectedNode->rect.wType = SableUI::RectType::FIXED;

		for (SableUI::Node* sibling : selectedNode->parent->children)
		{
			if (sibling->index != selectedNode->index)
			{
				sibling->rect.wType = SableUI::RectType::FILL;
				sibling->rect.hType = SableUI::RectType::FILL;
			}
		}

		CalculateNodeScales(selectedNode->parent);
	}
	else if (currentEdgeType == SableUI::NS_EDGE)
	{
		deltaPos.y = std::clamp(deltaPos.y,
			-oldNodeRect.h + minComponentSize,
			olderSiblingOldRect.h - minComponentSize);

		selectedNode->rect.h = oldNodeRect.h + deltaPos.y;
		olderSiblingNode->rect.h = olderSiblingOldRect.h - deltaPos.y;

		selectedNode->rect.hType = SableUI::RectType::FIXED;

		for (SableUI::Node* sibling : selectedNode->parent->children)
		{
			if (sibling->index != selectedNode->index)
			{
				sibling->rect.hType = SableUI::RectType::FILL;
				sibling->rect.wType = SableUI::RectType::FILL;
			}
		}

		CalculateNodeScales(selectedNode->parent);
	}

	CalculateNodePositions();
}

void SableUI::Window::CalculateNodePositions(Node* node)
{
	if (m_nodes.size() == 0) return;

	m_needsStaticRedraw = true;

	if (node == nullptr)
	{
		for (Node* child : m_root->children)
		{
			CalculateNodePositions(child);
		}
		return;
	}

	vec2 cursor = { 0, 0 };

	cursor.x += node->parent->rect.x;
	cursor.y += node->parent->rect.y;

	for (Node* sibling : node->parent->children)
	{
		if (sibling->index < node->index)
		{
			if (node->parent->type == NodeType::HSPLITTER)
			{
				cursor.x += sibling->rect.w;
			}
			else if (node->parent->type == NodeType::VSPLITTER)
			{
				cursor.y += sibling->rect.h;
			}
		}
		else break;
	}

	node->rect.x = cursor.x;
	node->rect.y = cursor.y;

	if (node->component)
	{
		node->component.get()->UpdateDrawable();
	}

	if (node->parent)
	{
		if (node->parent->component != nullptr)
		{
			if (auto* splitterComponent = dynamic_cast<SplitterComponent*>(node->component.get()))
			{
				splitterComponent->Render();
			}
		}
	}

	for (SableUI::Node* child : node->children)
	{
		CalculateNodePositions(child);
	}
}

void SableUI::Window::CalculateNodeScales(SableUI::Node* node)
{
	if (m_nodes.empty()) return;

	m_needsStaticRedraw = true;

	/* if first call, run for children of root node */
	if (node == nullptr)
	{
		for (SableUI::Node* child : m_root->children)
		{
			CalculateNodeScales(child);
		}
		return;
	}

	if (node->parent == nullptr) return;

	/* calculate rect using freaky logic */
	switch (node->parent->type)
	{
	case SableUI::NodeType::ROOTNODE:
		node->rect = node->parent->rect;
		break;

	case SableUI::NodeType::HSPLITTER:
	{
		node->rect.h = node->parent->rect.h;

		if (node->rect.wType == SableUI::RectType::FIXED) break;

		float wLeft = node->parent->rect.w;
		int amntOfFillSiblingsW = 0;

		for (SableUI::Node* sibling : node->parent->children)
		{
			if (sibling->rect.wType == SableUI::RectType::FILL)
			{
				amntOfFillSiblingsW++;
			}
			else
			{
				wLeft -= sibling->rect.w;
			}
		}

		if (amntOfFillSiblingsW > 0)
		{
			node->rect.w = wLeft / static_cast<float>(amntOfFillSiblingsW);
		}
		else
		{
			node->rect.w = wLeft;
		}

		if (wLeft <= minComponentSize * node->parent->children.size())
		{
			/* resize last child to min size */
			node->parent->children[node->parent->children.size() - 1]->rect.wType = SableUI::RectType::FIXED;
			node->parent->children[node->parent->children.size() - 1]->rect.w = minComponentSize;

			node->parent->children[node->parent->children.size() - 2]->rect.wType = SableUI::RectType::FILL;
		}

		break;
	}

	case SableUI::NodeType::VSPLITTER:
	{
		node->rect.w = node->parent->rect.w;

		if (node->rect.hType == SableUI::RectType::FIXED) break;

		float hLeft = node->parent->rect.h;
		int amntOfFillSiblingsH = 0;

		for (SableUI::Node* sibling : node->parent->children)
		{
			if (sibling->rect.hType == SableUI::RectType::FILL)
			{
				amntOfFillSiblingsH++;
			}
			else
			{
				hLeft -= sibling->rect.h;
			}
		}

		if (amntOfFillSiblingsH > 0)
		{
			node->rect.h = hLeft / static_cast<float>(amntOfFillSiblingsH);
		}
		else
		{
			node->rect.h = hLeft;
		}

		if (hLeft <= minComponentSize)
		{
			/* resize last child to min size */
			node->parent->children[node->parent->children.size() - 1]->rect.hType = SableUI::RectType::FIXED;
			node->parent->children[node->parent->children.size() - 1]->rect.h = minComponentSize;

			node->parent->children[node->parent->children.size() - 2]->rect.hType = SableUI::RectType::FILL;
		}

		break;
	}
	}

	/* recursively run for children */
	for (SableUI::Node* child : node->children)
	{
		CalculateNodeScales(child);
	}
}

//void SableUI::Window::MotionCallback(int x, int y)
//{
//	currentInstance->mousePos = ivec2(x, y);
//}
//
//void SableUI::Window::MouseButtonCallback(int button, int state, int x, int y)
//{
//	switch (button)
//	{
//	case GLUT_LEFT_BUTTON:
//		currentInstance->mouseButtonStates.LMB = static_cast<MouseState>(state); // dir translation
//		break;
//
//	case GLUT_RIGHT_BUTTON:
//		currentInstance->mouseButtonStates.RMB = static_cast<MouseState>(state); // dir translation
//		break;
//	}
//}
//
//void SableUI::Window::ReshapeCallback(int w, int h)
//{
//	currentInstance->windowSize = ivec2(w, h);
//	glViewport(0, 0, w, h);
//
//	currentInstance->renderer.renderTarget.Resize(w, h);
//
//	SetupRootNode(currentInstance->root, w, h);
//	currentInstance->RecalculateNodes();
//	currentInstance->RerenderAllNodes();
//
//	currentInstance->RecalculateNodes();
//	currentInstance->RerenderAllNodes();
//
//	currentInstance->needsStaticRedraw = true;
//}

SableUI::Window::~Window()
{
	for (SableUI::Node* node : m_nodes)
	{
		delete node;
	}
	m_nodes.clear();

	DestroyFontManager();
	DestroyDrawables();
	DestroyShaders();

	glfwDestroyWindow(m_window);
	glfwTerminate();

	SableUI_Log("Shut down successfully");
}