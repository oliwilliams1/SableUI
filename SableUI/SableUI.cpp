#define SDL_MAIN_HANDLED
#include "SableUI.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <functional>

#include "SBUI_Renderer.h"
#include "SBUI_Node.h"

static SDL_Window* window = nullptr;
static SDL_Surface* surface = nullptr;
static int frameDelay = 0;
static SableUI_node* root = nullptr;
static SDL_Cursor* cursorPointer = nullptr;
static SDL_Cursor* cursorNS = nullptr;
static SDL_Cursor* cursorEW = nullptr;
static bool resizing = false;

static std::vector<SableUI_node*> nodes;

void SableUI::CreateWindow(const std::string& title, int width, int height, int x, int y)
{
	if (window != nullptr || surface != nullptr)
	{
		printf("Window already created!\n");
		return;
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	int windowX = (x == -1) ? SDL_WINDOWPOS_CENTERED : x;
	int windowY = (y == -1) ? SDL_WINDOWPOS_CENTERED : y;

	window = SDL_CreateWindow(title.c_str(), windowX, windowY, width, height, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	}

	surface = SDL_GetWindowSurface(window);

	if (!surface)
	{
		printf("Surface could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	
	SetMaxFPS(60); // Default value

	Renderer::Init(surface);

	if (root != nullptr)
	{
		printf("Root node already created!\n");
		return;
	}

	cursorPointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	cursorNS      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursorEW = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);

	Renderer::Get().Clear({ 32, 32, 32 });

	root = new SableUI_node(NodeType::ROOTNODE, nullptr, "Root Node");
	SetupRootNode(root, width, height);
	nodes.push_back(root);
}

void SableUI::AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName)
{
	SableUI_node* parent = FindNodeByName(parentName);

	if (parent == nullptr)
	{
		printf("Parent node is null!\n");
		return;
	}

	if (parent->type == NodeType::COMPONENT)
	{
		printf("Cannot create a child node on a component node\n");
		return;
	}

	if (parent->type == NodeType::ROOTNODE && root->children.size() > 0)
	{
		printf("Root node can only have one child node\n");
		return;
	}

	SableUI_node* node = new SableUI_node(type, parent, name);

	if (node->parent->type == NodeType::ROOTNODE)
	{
		node->scaleFac.x = 1.0f;
		node->scaleFac.y = 1.0f;
	}

	if (node->parent->type == NodeType::VSPLITTER)
	{
		float scaleFac_y = 1.0f / node->parent->children.size();

		for (SableUI_node* child : node->parent->children)
		{
			child->scaleFac.x = 1.0f;
			child->scaleFac.y = scaleFac_y;
		}
	}

	if (node->parent->type == NodeType::HSPLITTER)
	{
		float scaleFac_x = 1.0f / node->parent->children.size();

		for (SableUI_node* child : node->parent->children)
		{
			child->scaleFac.x = scaleFac_x;
			child->scaleFac.y = 1.0f;
		}
	}

	nodes.push_back(node);
}

void SableUI::AttachComponentToNode(const std::string& nodeName, const BaseComponent& component)
{
	SableUI_node* node = FindNodeByName(nodeName);

	if (node == nullptr)
	{
		printf("Cannot find node: %s!\n", nodeName.c_str());
		return;
	}

	if (node->type != NodeType::COMPONENT)
	{
		printf("Node: %s is not a component node! Cannot attach component to a non-component node\n",
			nodeName.c_str());
		return;
	}

	if (node->component != nullptr)
	{
		printf("Node: %s already has a component attached!\n", nodeName.c_str());
		return;
	}
	node->component = std::make_unique<BaseComponent>(component);
	node->component.get()->parent = node;
}

bool SableUI::PollEvents()
{
	SDL_Event e;
	if (SDL_WaitEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			return false;
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				int w, h;
				SDL_GetWindowSize(window, &w, &h);

				surface = SDL_GetWindowSurface(window);
				Renderer::SetSurface(surface);
				Renderer::Get().Clear({ 32, 32, 32 });

				SetupRootNode(root, w, h);
				CalculateNodeDimensions();
				if (!surface)
				{
					printf("Surface could not be created! SDL_Error: %s\n", SDL_GetError());
				}
				break;
			}
		case SDL_MOUSEBUTTONUP:
			resizing = false;
			break;
		}
	}

	return true;
}

void SableUI::SetMaxFPS(int fps)
{
	frameDelay = 1000 / fps;
}

static void Resize(SableUI::ivec2 pos, SableUI_node* node = nullptr, SableUI::EdgeType edgeType = SableUI::EdgeType::NONE)
{
	static SableUI_node* selectedNode = nullptr;
	static SableUI::EdgeType currentEdgeType = SableUI::EdgeType::NONE;

	static SableUI::ivec2 oldPos = { 0, 0 };
	static SableUI::rect oldNodeRect = { 0, 0, 0, 0 };
	static SableUI::vec2 oldScaleFac = { 0, 0 };
	static float nParentsChildren = 1.0f; // prev div 0

	// First resize call fills data
	if (node != nullptr)
	{
		oldPos = pos;
		selectedNode = node;
		oldNodeRect = node->rect;
		oldScaleFac = node->scaleFac;
		currentEdgeType = edgeType;
		nParentsChildren = node->parent->children.size();

		return;
	}

	SableUI::ivec2 deltaPos = pos - oldPos;

	if (currentEdgeType == SableUI::EW_EDGE)
	{
		selectedNode->scaleFac.x = oldScaleFac.x + (deltaPos.x / (float)oldNodeRect.w) / nParentsChildren;
	}
	else if (currentEdgeType == SableUI::NS_EDGE)
	{
		selectedNode->scaleFac.y = oldScaleFac.y + (deltaPos.y / (float)oldNodeRect.h) / nParentsChildren;
	}

	SableUI::CalculateNodeDimensions();
}

void SableUI::Draw()
{
	uint32_t frameStart = SDL_GetTicks();

	Renderer& renderer = Renderer::Get();

	ivec2 cursorPos = { 0, 0 };
	uint32_t mouseState = SDL_GetMouseState(&cursorPos.x, &cursorPos.y);

	static SDL_Cursor* currentCursor = cursorPointer;
	static bool resCalled = false;

	for (SableUI_node* node : nodes)
	{
		if (node->type != NodeType::COMPONENT || node->component == nullptr)
		{
			continue;
		}

		if (!RectBoundingBox(node->rect, cursorPos))
{
			continue;
		}

		node->component.get()->Render();

		EdgeType edgeType = NONE;
		float d1 = DistToEdge(node->rect, cursorPos, edgeType);
		SDL_Cursor* cursorToSet = cursorPointer;

		EdgeType e = NONE;
		if (d1 < 5.0f && DistToEdge(root->rect, cursorPos, e) > 5.0f)
		{
			switch (edgeType)
			{
			case NS_EDGE:
				if (node->parent->type == NodeType::VSPLITTER)
				{
					cursorToSet = cursorNS;
				}
				break;
			case EW_EDGE:
				if (node->parent->type == NodeType::HSPLITTER)
				{
					cursorToSet = cursorEW;
				}
				break;
			}
		}

		if (!resizing && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && cursorToSet != cursorPointer)
		{
			resCalled = true;

			int indexYoungerSibling = node->index - 1;
			if (indexYoungerSibling < 0)
			{
				Resize(cursorPos, node, edgeType);
				resizing = true;
				continue;
			}

			SableUI_node* youngerSibling = node->parent->children[indexYoungerSibling];
			float d2 = DistToEdge(youngerSibling->rect, cursorPos, edgeType);

			if (d2 < 10.0f)
			{
				Resize(cursorPos, youngerSibling, edgeType);
				resizing = true;
				continue;
			}
			else
			{
				Resize(cursorPos, node, edgeType);
			}
		}

		if (currentCursor != cursorToSet && !resizing)
		{
			SDL_SetCursor(cursorToSet);
			currentCursor = cursorToSet;
		}
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

	renderer.Draw();
	SDL_UpdateWindowSurface(window);

	uint32_t frameTime = SDL_GetTicks() - frameStart;
	if (frameTime < static_cast<uint32_t>(frameDelay))
	{
		SDL_Delay(frameDelay - frameTime);
	}
}

static void PrintNode(SableUI_node* node, int depth = 0)
{
	if (node == nullptr) return;

	std::string indent(depth * 2, ' ');

	size_t nameLength = node->name.length();
	int spacesNeeded = 24 - (int)nameLength;

	if (spacesNeeded < 0) spacesNeeded = 0;

	printf("name: %s scaleFac: (%.2f x %.2f), sizePx: (%.2f x %.2f), pos: (%.2f x %.2f), index: %i\n",
		node->name.c_str(), node->scaleFac.x, node->scaleFac.y, 
		node->rect.w, node->rect.h, node->rect.x, node->rect.y,
		node->index);

	for (SableUI_node* child : node->children)
	{
		PrintNode(child, depth + 1);
	}
}

void SableUI::PrintNodeTree()
{
	PrintNode(root);
}

SableUI_node* SableUI::GetRoot()
{
	return root;
}

SableUI_node* SableUI::FindNodeByName(const std::string& name)
{
	for (SableUI_node* node : nodes)
	{
		if (node->name == name)
		{
			return node;
		}
	}

	return nullptr;
}

void SableUI::CalculateNodeDimensions(SableUI_node* node)
{
	if (node == nullptr)
	{
		for (SableUI_node* child : root->children)
		{
			CalculateNodeDimensions(child);
		}
		return;
	}

	node->rect.w = node->parent->rect.w * node->scaleFac.x;
	node->rect.h = node->parent->rect.h * node->scaleFac.y;

	vec2 cursor = { 0, 0 };

	cursor.x += node->parent->rect.x;
	cursor.y += node->parent->rect.y;

	for (SableUI_node* sibling : node->parent->children)
	{
		if (sibling->index < node->index)
		{
			if (node->parent->type == NodeType::HSPLITTER)
			{
				cursor.x += sibling->rect.w * node->parent->scaleFac.x;
			}
			else if (node->parent->type == NodeType::VSPLITTER)
			{
				cursor.y += sibling->rect.h * node->parent->scaleFac.y;
			}
		}
		else break;
	}

	node->rect.x = cursor.x;
	node->rect.y = cursor.y;

	if (node->component)
	{
		node->component.get()->Render();
	}

	for (SableUI_node* child : node->children)
	{
		CalculateNodeDimensions(child);
	}
}

void SableUI::Destroy()
{
	if (window == nullptr || surface == nullptr)
	{
		printf("Destroying when window or surface not created!\n");
		return;
	}

	for (SableUI_node* node : nodes)
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