#define SDL_MAIN_HANDLED
#include "SableUI.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

#include "node.h"

static SDL_Window* window = nullptr;
static SDL_Surface* surface = nullptr;
static int frameDelay = 0;
static SBUI_node* root = nullptr;

static std::vector<SBUI_node*> nodes;

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


	if (root != nullptr)
	{
		printf("Root node already created!\n");
		return;
	}

	root = new SBUI_node(NodeType::ROOTNODE, nullptr, "Root Node", 0);
	SetupRootNode(root, width, height);
	nodes.push_back(root);
}

void SableUI::AddNodeToParent(NodeType type, const std::string& name, SBUI_node* parent)
{
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

	SBUI_node* node = new SBUI_node(type, parent, name, nodes.size() + 1);

	if (node->parent->type == NodeType::ROOTNODE)
	{
		node->wFac = 1.0f;
		node->hFac = 1.0f;
	}

	if (node->parent->type == NodeType::VSPLITTER)
	{
		float hFac = 1.0f / node->parent->children.size();

		for (SBUI_node* child : node->parent->children)
		{
			child->wFac = 1.0f;
			child->hFac = hFac;
		}
	}

	if (node->parent->type == NodeType::HSPLITTER)
	{
		float wFac = 1.0f / node->parent->children.size();

		for (SBUI_node* child : node->parent->children)
		{
			child->wFac = wFac;
			child->hFac = 1.0f;
		}
	}

	nodes.push_back(node);
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
	Uint32 frameStart = SDL_GetTicks();

	if (SDL_LockSurface(surface) < 0) {
		printf("Unable to lock surface! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	Uint32 color = SDL_MapRGB(surface->format, 32, 32, 32);
	SDL_FillRect(surface, nullptr, color);

	SDL_UnlockSurface(surface);

	SDL_UpdateWindowSurface(window);

	Uint32 frameTime = SDL_GetTicks() - frameStart;
	if (frameTime < frameDelay) {
		SDL_Delay(frameDelay - frameTime);
	}
}

void SableUI::Destroy()
{
	if (window == nullptr || surface == nullptr)
	{
		printf("Destroying when window or surface not created!\n");
		return;
	}

	for (SBUI_node* node : nodes)
	{
		delete node;
	}
	nodes.clear();

	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
		surface = nullptr;
	}

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}

static void PrintNode(SBUI_node* node, int depth = 0)
{
	if (node == nullptr) return;

	std::string indent(depth * 2, ' ');

	int nameLength = node->name.length();
	int spacesNeeded = 24 - nameLength;

	if (spacesNeeded < 0) spacesNeeded = 0;

	std::cout << indent << node->name;
	std::cout << std::string(spacesNeeded, ' ');
	printf("%fx%f\n", node->wFac, node->hFac);

	for (SBUI_node* child : node->children)
	{
		PrintNode(child, depth + 1);
	}
}

void SableUI::PrintNodeTree()
{
	PrintNode(root);
}

SBUI_node* SableUI::GetRoot()
{
	return root;
}

SBUI_node* SableUI::FindNodeByName(const std::string& name)
{
	for (SBUI_node* node : nodes)
	{
		if (node->name == name)
		{
			return node;
		}
	}

	return nullptr;
}