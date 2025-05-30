#pragma once
#define SDL_MAIN_HANDLED
#include <string>
#include <chrono>
#include "SableUI/node.h"

namespace SableUI
{
	enum class MouseState
	{
		DOWN = 0x0000,
		UP   = 0x0001
	};

	struct MouseButtonState
	{
		MouseState LMB = MouseState::UP;
		MouseState RMB = MouseState::UP;
	};

	class Window
	{
	public:
		Window(int argc, char** argv, const std::string& title, int width, int height, int x = -1, int y = -1);

		bool PollEvents();
		void Draw();
		bool needsStaticRedraw = false;
		void SetMaxFPS(int fps);

		void PrintNodeTree();
		void SetupSplitter(const std::string& name, float bSize);
		void AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName);
		void AttachComponentToNode(const std::string& nodeName, std::unique_ptr<BaseComponent> component);
		void AddElementToComponent(const std::string& nodeName, const ElementInfo& info);
		void AddElementToElement(const std::string& elementName, const ElementInfo& info);

		Node* GetRoot();
		Node* FindNodeByName(const std::string& name);

		void OpenUIFile(const std::string& path);
		
		void RerenderAllNodes();
		void RecalculateNodes();

		~Window();

	private:
		void CalculateNodePositions(Node* node = nullptr);
		void CalculateNodeScales(Node* node = nullptr);
		void Resize(vec2 pos, Node* node = nullptr);

		std::chrono::milliseconds frameDelay;
		Node* root = nullptr;
		bool resizing = false;
		Texture surface;
		std::vector<Node*> nodes;

		static void MotionCallback(int x, int y);
		ivec2 mousePos = ivec2(0, 0);

		static void MouseButtonCallback(int button, int state, int x, int y);
		MouseButtonState mouseButtonStates;

		static void ReshapeCallback(int width, int height);
		ivec2 windowSize = ivec2(0, 0);
		
		static Window* currentInstance;
		int windowID = -1;
	};
}