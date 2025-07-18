#pragma once
#define SDL_MAIN_HANDLED
#include <string>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

		ivec2 m_mousePos = ivec2(0, 0);
		MouseButtonState m_mouseButtonStates;
		ivec2 m_windowSize = ivec2(0, 0);

	private:
		int GetRefreshRate();
		Renderer m_renderer;

		void CalculateNodePositions(Node* node = nullptr);
		void CalculateNodeScales(Node* node = nullptr);
		void CalculateAllNodeMinimumBounds();
		void Resize(vec2 pos, Node* node = nullptr);
		void DrawDebugBounds();
		void DrawDebugElementBounds(Node* node = nullptr);

		std::chrono::milliseconds m_frameDelay;
		Node* m_root = nullptr;
		bool m_resizing = false;
		std::vector<Node*> m_nodes;

		static void MousePosCallback(GLFWwindow* window, double x, double y);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void ResizeCallback(GLFWwindow* window, int width, int height);

		int CalculateMinimumWidth(Node* node);
		int CalculateMinimumHeight(Node* node);
		
		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		vec2 windowDPI = vec2(96.0f, 96.0f);

		GLFWcursor* m_currentCursor = nullptr;
		GLFWcursor* m_arrowCursor   = nullptr;
		GLFWcursor* m_hResizeCursor = nullptr;
		GLFWcursor* m_vResizeCursor = nullptr;
	};
}