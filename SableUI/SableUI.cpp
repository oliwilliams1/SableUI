#define SDL_MAIN_HANDLED
#include "SableUI/SableUI.h"
#include "SableUI/renderer.h"
#include "SableUI/node.h"

#include <SDL2/SDL.h>
#include <tinyxml2.h>
#include <cstdio>
#include <iostream>
#include <functional>
#include <algorithm>
#include <stack>

#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")
#include <SDL_syswm.h>
#include <windows.h>
#include <dwmapi.h>
#endif

constexpr float minComponentSize = 20.0f;

static SDL_Window* window = nullptr;
static SDL_Surface* surface = nullptr;
static int frameDelay = 0;
static SableUI::Node* root = nullptr;
static SDL_Cursor* cursorPointer = nullptr;
static SDL_Cursor* cursorNS = nullptr;
static SDL_Cursor* cursorEW = nullptr;
static bool resizing = false;

static std::vector<SableUI::Node*> nodes;

static void CalculateNodeScales(SableUI::Node* node = nullptr)
{
	if (nodes.empty()) return;

	/* if first call, run for children of root node */
	if (node == nullptr)
	{
		for (SableUI::Node* child : root->children)
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

static void Resize(SableUI::vec2 pos, SableUI::Node* node = nullptr)
{
	/* static for multiple calls (lifetime of static is until mouse up) */
	static SableUI::Node* selectedNode = nullptr;
	static SableUI::EdgeType currentEdgeType = SableUI::EdgeType::NONE;

	static SableUI::vec2 oldPos = { 0, 0 };
	static SableUI::rect oldNodeRect = { 0, 0, 0, 0 };
	static float nParentsChildren = 1.0f; // prev div 0

	static SableUI::Node* olderSiblingNode = nullptr;
	static SableUI::rect olderSiblingOldRect = { 0, 0, 0, 0 };

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

	SableUI::CalculateNodePositions();
}

static float DistToEdge(SableUI::Node* node, SableUI::ivec2 p)
{
	SableUI::rect r = node->rect;
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

static void RerenderAllNodes()
{
	for (SableUI::Node* node : nodes)
	{
		if (node->component)
		{
			node->component.get()->UpdateDrawable();
		}
	}
}

void SableUI::SetupSplitter(const std::string& name, float bSize)
{
	SableUI::Node* node = SableUI::FindNodeByName(name);

	if (node == nullptr) return;

	node->bSize = bSize;
}

void SableUI::SBCreateWindow(const std::string& title, int width, int height, int x, int y)
{
	/* prevent func from being called twice */
	if (window != nullptr || surface != nullptr)
	{
		SableUI_Log("Window already created!");
		return;
	}


	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SableUI_Error("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return;
	}

	int windowX = (x == -1) ? SDL_WINDOWPOS_CENTERED : x;
	int windowY = (y == -1) ? SDL_WINDOWPOS_CENTERED : y;

	window = SDL_CreateWindow(title.c_str(), windowX, windowY, width, height, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		SableUI_Error("Window could not be created! SDL_Error: %s", SDL_GetError());
	}

#ifdef _WIN32
	/* enable immersive darkmode on windows via api */
	SDL_SysWMinfo sysInfo{};

	SDL_VERSION(&sysInfo.version);
	SDL_GetWindowWMInfo(window, &sysInfo);

	HWND hwnd = sysInfo.info.win.window;

	BOOL dark_mode = true;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));

	// Force update of immersive dark mode
	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);
#endif

	surface = SDL_GetWindowSurface(window);

	if (!surface)
	{
		SableUI_Error("Surface could not be created! SDL_Error: %s", SDL_GetError());
	}
	
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0)
	{
		SableUI_Error("Failed to get display mode! SDL_Error: %s", SDL_GetError());
	}
	else
	{
		/* set internal max hz to display refresh rate */
		SetMaxFPS(displayMode.refresh_rate);
	}

	Renderer::Init(surface);

	if (root != nullptr)
	{
		SableUI_Error("Root node already created!");
		return;
	}

	/* init cursors for resizing */
	cursorPointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	cursorNS      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursorEW      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);

	/* make root node */
	root = new SableUI::Node(NodeType::ROOTNODE, nullptr, "Root Node");
	SetupRootNode(root, width, height);
	nodes.push_back(root);
}

void SableUI::AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName)
{
	CalculateNodePositions();

	SableUI::Node* parent = FindNodeByName(parentName);

	if (parent == nullptr)
	{
		SableUI_Error("Parent node is null!");
		return;
	}

	if (parent->type == NodeType::COMPONENT)
	{
		SableUI_Error("Cannot create a child node on a component node");
		return;
	}

	if (parent->type == NodeType::ROOTNODE && root->children.size() > 0)
	{
		SableUI_Error("Root node can only have one child node");
		return;
	}

	SableUI::Node* node = new SableUI::Node(type, parent, name);

	nodes.push_back(node);
}

void SableUI::AttachComponentToNode(const std::string& nodeName, std::unique_ptr<SableUI::BaseComponent> component)
{
	SableUI::Node* node = FindNodeByName(nodeName);

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
}

void SableUI::AddElementToComponent(const std::string& nodeName, std::unique_ptr<BaseElement> element)
{
	SableUI::Node* node = FindNodeByName(nodeName);

	if (node == nullptr)
	{
		SableUI_Warn("Cannot find node: %s!", nodeName.c_str());
		return;
	}

	if (node->type == NodeType::COMPONENT)
	{
		if (auto* defaultComponent = dynamic_cast<DefaultComponent*>(node->component.get()))
		{
			defaultComponent->AddElement(element);
		}
	}
	else
	{
		SableUI_Error("Adding element to non-component node: %s", nodeName.c_str());
	}
}

static bool init = false;

bool SableUI::PollEvents()
{
	if (!init)
	{
		RecalculateNodes(); RerenderAllNodes(); init = true;
	}

	SDL_Event e;
	if (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			return false;
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
				|| e.window.event == SDL_WINDOW_MINIMIZED || e.window.event == SDL_WINDOW_MAXIMIZED)
			{
				int w, h;
				SDL_GetWindowSize(window, &w, &h);

				surface = SDL_GetWindowSurface(window);
				Renderer::SetSurface(surface);

				SetupRootNode(root, w, h);
				CalculateNodeScales();
				CalculateNodePositions();

				Draw();
				RerenderAllNodes();

				if (!surface)
				{
					SableUI_Runtime_Error("Surface could not be created! SDL_Error: %s", SDL_GetError());
				}
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			resizing = false;
			RerenderAllNodes();
			break;
		}
	}

	return true;
}

void SableUI::SetMaxFPS(int fps)
{
	frameDelay = 1000 / fps;
}

void SableUI::Draw()
{
	uint32_t frameStart = SDL_GetTicks();

	Renderer& renderer = Renderer::Get();

	/* mouse state for resizing logic */
	ivec2 cursorPos = { 0, 0 };
	uint32_t mouseState = SDL_GetMouseState(&cursorPos.x, &cursorPos.y);

	/* static for multiple calls on one resize event (lifetime of static is until mouse up) */
	static SDL_Cursor* currentCursor = cursorPointer;
	static bool resCalled = false;

	SDL_Cursor* cursorToSet = cursorPointer;

	/* check if need to resize */
	for (SableUI::Node* node : nodes)
	{
		if (!RectBoundingBox(node->rect, cursorPos)) continue;

		if (cursorPos.x == 0 && cursorPos.y == 0) continue;
	
		float d1 = DistToEdge(node, cursorPos);

		if (node->parent == nullptr) continue;

		if (d1 < 5.0f)
		{
			switch (node->parent->type)
			{
			case NodeType::VSPLITTER:
				cursorToSet = cursorNS;
				break;

			case NodeType::HSPLITTER:
				cursorToSet = cursorEW;
				break;
			}

			if (!resizing && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && cursorToSet != cursorPointer)
			{
				resCalled = true;

				Resize(cursorPos, node);

				resizing = true;
				continue;
			}
		}
	}

	if (currentCursor != cursorToSet && !resizing)
	{
		SDL_SetCursor(cursorToSet);
		currentCursor = cursorToSet;
	}

	if (resizing)
	{
		if (!resCalled)
		{
			Resize(cursorPos);
			if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
			{
				resizing = false;
			}
		}
		else
		{
			resCalled = false;
		}
	}

	/* flush drawable stack & render to screen */
	renderer.Draw();
	SDL_UpdateWindowSurface(window);

	/* ensure application doesnt run faster than desired fps */
	uint32_t frameTime = SDL_GetTicks() - frameStart;
	if (frameTime < static_cast<uint32_t>(frameDelay))
	{
		SDL_Delay(frameDelay - frameTime);
	}
}

void SableUI::PrintNodeTree()
{
	PrintNode(root);
}

SableUI::Node* SableUI::GetRoot()
{
	return root;
}

SableUI::Node* SableUI::FindNodeByName(const std::string& name)
{
	for (SableUI::Node* node : nodes)
	{
		if (node->name == name)
		{
			return node;
		}
	}

	return nullptr;
}

void SableUI::RecalculateNodes()
{
	CalculateNodeScales();
	CalculateNodePositions();

	for (SableUI::Node* node : nodes)
	{
		if (node->component != nullptr && node->type != NodeType::COMPONENT)
		{
			node->component.get()->UpdateDrawable();
		}
	}

	RerenderAllNodes();
}

void SableUI::CalculateNodePositions(SableUI::Node* node)
{
	if (nodes.size() == 0) return;

	if (node == nullptr)
	{
		for (SableUI::Node* child : root->children)
		{
			CalculateNodePositions(child);
		}
		return;
	}

	vec2 cursor = { 0, 0 };

	cursor.x += node->parent->rect.x;
	cursor.y += node->parent->rect.y;

	for (SableUI::Node* sibling : node->parent->children)
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

void SableUI::Destroy()
{
	if (window == nullptr || surface == nullptr)
	{
		SableUI_Error("Destroying when window or surface not created!");
		return;
	}

	for (SableUI::Node* node : nodes)
	{
		delete node;
	}
	nodes.clear();

	Renderer::Shutdown();

	SDL_FreeCursor(cursorPointer);
	SDL_FreeCursor(cursorNS);
	SDL_FreeCursor(cursorEW);

	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
		surface = nullptr;
	}

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}

void SableUI::OpenUIFile(const std::string& path)
{
	if (nodes.size() > 0)
	{
		for (SableUI::Node* node : nodes)
		{
			delete node;
		}
		nodes.clear();
		root = nullptr;
		SableUI_Log("Reloading UI");
	}

	using namespace tinyxml2;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
	{
		SableUI_Error("Failed to load file: %s", path.c_str());
		return;
	}

	XMLElement* rootElement = doc.FirstChildElement("root");
	if (!rootElement) {
		SableUI_Error("Invalid XML structure: Missing <root> element");
		return;
	}

	root = new SableUI::Node(NodeType::ROOTNODE, nullptr, "Root Node");
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	SetupRootNode(root, w, h);
	nodes.push_back(root);

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
			SableUI::colour colour = SableUI::colour(51, 51, 51);
			if (colourAttr) colour = SableUI::StringTupleToColour(colourAttr);

			const char* borderAttr = element->Attribute("border");
			float border = 1.0f;
			if (borderAttr) border = std::stof(borderAttr);

			const char* borderColourAttr = element->Attribute("borderColour");
			SableUI::colour borderColour = SableUI::StringTupleToColour(borderColourAttr);

			std::string nodeName = (nameAttr) ? nameAttr : elementName + " " + std::to_string(nodes.size());

			if (elementName == "hsplitter")
			{
				SableUI::AddNodeToParent(NodeType::HSPLITTER, nodeName, parentName);
				SableUI::SetupSplitter(nodeName, border);
				SableUI::AttachComponentToNode(nodeName, std::make_unique<SplitterComponent>(colour));

				parentStack.push(nodeName);
				parseNode(element->FirstChildElement(), nodeName);
				parentStack.pop();
			}
			else if (elementName == "vsplitter")
			{
				SableUI::AddNodeToParent(NodeType::VSPLITTER, nodeName, parentName);
				SableUI::SetupSplitter(nodeName, border);
				SableUI::AttachComponentToNode(nodeName, std::make_unique<SplitterComponent>(colour));

				parentStack.push(nodeName);
				parseNode(element->FirstChildElement(), nodeName);
				parentStack.pop();
			}
			else if (elementName == "component")
			{
				SableUI::AddNodeToParent(NodeType::COMPONENT, nodeName, parentName);
				SableUI::AttachComponentToNode(nodeName, std::make_unique<DefaultComponent>(colour));
			}

			element = element->NextSiblingElement();
		}
	};

	parseNode(rootElement->FirstChildElement(), parentStack.top());
}