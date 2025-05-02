#define SDL_MAIN_HANDLED
#include "SableUI.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

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

	root = new SbUI_node(NodeType::ROOTNODE, nullptr, "Root Node", 0);
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

	SbUI_node* node = new SbUI_node(type, parent, name, (int)nodes.size() + 1);

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

	CaclulateNodeDimensions(root);
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
	printf("%fx%f, %ix%i\n", node->wFac, node->hFac, node->wPx, node->hPx);

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

void SableUI::CaclulateNodeDimensions(SbUI_node* node)
{
	if (node == nullptr) return;

	if (node->parent) {
		node->wPx = static_cast<uint16_t>(node->parent->wPx * node->wFac);
		node->hPx = static_cast<uint16_t>(node->parent->hPx * node->hFac);

		node->xPx = static_cast<uint16_t>(node->parent->wPx - node->wPx);
		node->yPx = 0;
	}

	for (SbUI_node* child : node->children) {
		SableUI::CaclulateNodeDimensions(child);
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