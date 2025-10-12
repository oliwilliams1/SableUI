#pragma once
#pragma warning(push)
#pragma warning(disable : 4005)
#include <string>
#include <chrono>
#include <vector>

#include "SableUI/panel.h"
#pragma warning(pop)

struct GLFWcursor;
struct GLFWwindow;

namespace SableUI
{
	void SableUI_Window_Initalise_GLFW();
	void SableUI_Window_Terminate_GLFW();
	void* GetCurrentContext();

	struct ResizeState
	{
		BasePanel* selectedPanel = nullptr;
		EdgeType currentEdgeType = EdgeType::NONE;
		Rect oldPanelRect = { 0,0,0,0 };
		BasePanel* olderSiblingNode = nullptr;
		Rect olderSiblingOldRect = { 0,0,0,0 };
		ivec2 prevPos = { 0,0 };
		ivec2 totalDelta = { 0,0 };

		ivec2 oldPos = { 0,0 };
		ivec2 pendingDelta = { 0,0 };
	};

	class Window
	{
	public:
		Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, int x = -1, int y = -1);
		~Window();

		bool PollEvents();
		void Draw();
		bool m_needsStaticRedraw = false;
		void SetMaxFPS(int fps);

		RootPanel* GetRoot();

		void RerenderAllNodes();
		void RecalculateNodes();

		ivec2 m_mousePos = ivec2(0, 0);
		MouseButtonState m_mouseButtonStates;
		ivec2 m_windowSize = ivec2(0, 0);
		bool m_LayoutUpdated = true;

	private:
		void InitOpenGL();
		void InitVulkan();

		int GetRefreshRate();
		Renderer m_renderer;

		void Resize(ivec2 pos, BasePanel* node = nullptr);
		GLFWcursor* CheckResize(BasePanel* node, bool* resCalled);
		void ResizeStep(SableUI::ivec2 deltaPos, SableUI::BasePanel* panel, SableUI::BasePanel* root);
		ResizeState m_resizeState;

		std::chrono::milliseconds m_frameDelay;
		RootPanel* m_root = nullptr;
		bool m_resizing = false;

		static void MousePosCallback(GLFWwindow* window, double x, double y);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void ResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		vec2 windowDPI = vec2(96.0f, 96.0f);

		GLFWcursor* m_currentCursor = nullptr;
		GLFWcursor* m_arrowCursor = nullptr;
		GLFWcursor* m_hResizeCursor = nullptr;
		GLFWcursor* m_vResizeCursor = nullptr;

		bool mouseMoved	= false;
		bool mouseEvent = false;
	};
}