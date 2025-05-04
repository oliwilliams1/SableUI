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

	window = SDL_CreateWindow(title.c_str(), windowX, windowY, width, height, SDL_WINDOW_SHOWN);
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

	SbUI_Renderer::Init(surface);

	if (root != nullptr)
	{
		printf("Root node already created!\n");
		return;
	}

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
		node->wFac = 1.0f;
		node->hFac = 1.0f;
	}

	if (node->parent->type == NodeType::VSPLITTER)
	{
		float hFac = 1.0f / node->parent->children.size();

		for (SbUI_node* child : node->parent->children)
		{
			child->wFac = 1.0f;
			child->hFac = hFac;
		}
	}

	if (node->parent->type == NodeType::HSPLITTER)
	{
		float wFac = 1.0f / node->parent->children.size();

		for (SbUI_node* child : node->parent->children)
		{
			child->wFac = wFac;
			child->hFac = 1.0f;
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
		printf("Node: %s is not a component node! Cannot attach component to a non-component node\n", nodeName.c_str());
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
		default:
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

	SbUI_Renderer& renderer = SbUI_Renderer::Get();

	renderer.Clear({ 32, 32, 32 });

	for (SbUI_node* node : nodes)
	{
		if (node->type == NodeType::COMPONENT && node->component != nullptr)
		{
			node->component.get()->Render();
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

	std::cout << indent << node->name;
	std::cout << std::string(spacesNeeded, ' ');
	printf("%fx%f, size %ix%i, pos: %ix%i, index: %i\n", node->wFac, node->hFac, node->wPx, node->hPx, node->xPx, node->yPx, node->index);

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

	node->wPx = static_cast<uint16_t>(node->parent->wPx * node->wFac);
	node->hPx = static_cast<uint16_t>(node->parent->hPx * node->hFac);

	SbUIvec2 cursor = { 0, 0 };

	cursor.x += static_cast<float>(node->parent->xPx);
	cursor.y += static_cast<float>(node->parent->yPx);

	// Reset cursor for the current node calculation
	for (SbUI_node* sibling : node->parent->children)
	{
		if (sibling->index < node->index)
		{
			if (node->parent->type == NodeType::HSPLITTER)
			{
				cursor.x += static_cast<float>(sibling->wPx * node->parent->wFac);
			}
			else if (node->parent->type == NodeType::VSPLITTER)
			{
				cursor.y += static_cast<float>(sibling->hPx * node->parent->hFac);
			}
		}
		else break; // Stop when we reach the current node
	}

	node->xPx = static_cast<uint16_t>(cursor.x);
	node->yPx = static_cast<uint16_t>(cursor.y);

	std::cout << "Node: " << node->name << " x: " << node->xPx << ", y: " << node->yPx << ", w: " << node->wPx << ", h: " << node->hPx << std::endl;

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

	SbUI_Renderer::Shutdown();

	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
		surface = nullptr;
	}

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}