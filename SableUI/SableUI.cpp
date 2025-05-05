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
static SbUI_node* root = nullptr;

static std::vector<SbUI_node*> nodes;

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

	Renderer::Get().Clear({ 32, 32, 32 });

	root = new SbUI_node(NodeType::ROOTNODE, nullptr, "Root Node");
	SetupRootNode(root, width, height);
	nodes.push_back(root);
}

void SableUI::AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName)
{
	SbUI_node* parent = FindNodeByName(parentName);

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

	SbUI_node* node = new SbUI_node(type, parent, name);

	if (node->parent->type == NodeType::ROOTNODE)
	{
		node->scaleFac.x = 1.0f;
		node->scaleFac.y = 1.0f;
	}

	if (node->parent->type == NodeType::VSPLITTER)
	{
		float scaleFac_y = 1.0f / node->parent->children.size();

		for (SbUI_node* child : node->parent->children)
		{
			child->scaleFac.x = 1.0f;
			child->scaleFac.y = scaleFac_y;
		}
	}

	if (node->parent->type == NodeType::HSPLITTER)
	{
		float scaleFac_x = 1.0f / node->parent->children.size();

		for (SbUI_node* child : node->parent->children)
		{
			child->scaleFac.x = scaleFac_x;
			child->scaleFac.y = 1.0f;
		}
	}

	nodes.push_back(node);
}

void SableUI::AttachComponentToNode(const std::string& nodeName, const BaseComponent& component)
{
	SbUI_node* node = FindNodeByName(nodeName);

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

	ivec2 cursorPos = { 0, 0 };
	SDL_GetMouseState(&cursorPos.x, &cursorPos.y);

	for (SbUI_node* node : nodes)
	{
		if (node->type == NodeType::COMPONENT && node->component != nullptr)
		{
			if (RectBoundingBox(node->rect, cursorPos))
			{
				node->component.get()->Render();

				EdgeType edgeType = NONE;
				float d = DistToEdge(node->rect, cursorPos, edgeType);

				if (d < 5.0f)
				{
					std::cout << "hovering over edge: " << node->name << std::endl;
				}
			}
		}
	}

	renderer.Draw();

	SDL_UpdateWindowSurface(window);

	uint32_t frameTime = SDL_GetTicks() - frameStart;
	if ((int)frameTime < frameDelay) {
		SDL_Delay(frameDelay - frameTime);
	}
}

static void PrintNode(SbUI_node* node, int depth = 0)
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

	for (SbUI_node* child : node->children)
	{
		PrintNode(child, depth + 1);
	}
}

void SableUI::PrintNodeTree()
{
	PrintNode(root);
}

SbUI_node* SableUI::GetRoot()
{
	return root;
}

SbUI_node* SableUI::FindNodeByName(const std::string& name)
{
	for (SbUI_node* node : nodes)
	{
		if (node->name == name)
		{
			return node;
		}
	}

	return nullptr;
}

void SableUI::CalculateNodeDimensions(SbUI_node* node)
{
	if (node == nullptr)
	{
		for (SbUI_node* child : root->children)
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

	for (SbUI_node* sibling : node->parent->children)
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

	for (SbUI_node* child : node->children)
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

	for (SbUI_node* node : nodes)
	{
		delete node;
	}
	nodes.clear();

	Renderer::Shutdown();

	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
		surface = nullptr;
	}

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}