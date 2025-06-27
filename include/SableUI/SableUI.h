#pragma once
#define SDL_MAIN_HANDLED
#include <string>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SableUI/node.h"
#include "SableUI/renderer.h"

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

	void Initialize(int argc, char** argv);

	class Window
	{
	public:
		Window(const std::string& title, int width, int height, int x = -1, int y = -1);

		bool PollEvents();
		void Draw();
		bool m_needsStaticRedraw = false;
		void SetMaxFPS(int fps);

		void PrintNodeTree();
		void SetupSplitter(const std::string& name, float bSize);
		Node* AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName);
		void AttachComponentToNode(const std::string& nodeName, std::unique_ptr<BaseComponent> component);
		Element* AddElementToComponent(const std::string& nodeName, ElementInfo& info, ElementType type);
		Element* AddElementToElement(const std::string& elementName, ElementInfo& info, ElementType type);

		Node* GetRoot();
		Node* FindNodeByName(const std::string& name);

		void OpenUIFile(const std::string& path);
		
		void RerenderAllNodes();
		void RecalculateNodes();

		~Window();

	private:
		Renderer renderer;

		void CalculateNodePositions(Node* node = nullptr);
		void CalculateNodeScales(Node* node = nullptr);
		void Resize(vec2 pos, Node* node = nullptr);

		std::chrono::milliseconds m_frameDelay;
		Node* m_root = nullptr;
		bool m_resizing = false;
		std::vector<Node*> m_nodes;

		static void MotionCallback(int x, int y);
		ivec2 m_mousePos = ivec2(0, 0);

		static void MouseButtonCallback(int button, int state, int x, int y);
		MouseButtonState m_mouseButtonStates;

		static void ReshapeCallback(int width, int height);
		ivec2 m_windowSize = ivec2(0, 0);
		
		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;

		GLFWcursor* m_currentCursor = nullptr;
		GLFWcursor* m_arrowCursor   = nullptr;
		GLFWcursor* m_hResizeCursor = nullptr;
		GLFWcursor* m_vResizeCursor = nullptr;
	};
}