#pragma once
#pragma warning(push)
#pragma warning(disable : 4005)
#include <string>
#include <chrono>
#include <vector>
#include <array>

#include "SableUI/renderer.h"
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
		Rect oldPanelRect = { 0, 0, 0, 0 };
		BasePanel* olderSiblingNode = nullptr;
		Rect olderSiblingOldRect = { 0, 0, 0, 0 };
		ivec2 prevPos = { 0, 0 };
		ivec2 totalDelta = { 0, 0 };

		ivec2 oldPos = { 0, 0 };
		ivec2 pendingDelta = { 0, 0 };
	};

	class Window
	{
	public:
		Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, int x = -1, int y = -1);
		~Window();

		bool PollEvents();
		void Draw();
		bool m_needsStaticRedraw = false;
		bool m_needsRefresh = false;

		RootPanel* GetRoot();

		void RerenderAllNodes();
		void RecalculateNodes();

		UIEventContext ctx;
		ivec2 m_windowSize = { 0, 0 };

	private:
		RendererBackend* m_renderer = nullptr;

		void HandleResize();
		GLFWcursor* CheckResize(BasePanel* node, bool* resCalled);
		void Resize(ivec2 pos, BasePanel* panel = nullptr);
		void ResizeStep(ivec2 deltaPos, BasePanel* panel, BasePanel* root);
		ResizeState m_resizeState;

		RootPanel* m_root = nullptr;
		bool m_resizing = false;

		static void MousePosCallback(GLFWwindow* window, double x, double y);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void ResizeCallback(GLFWwindow* window, int width, int height);
		static void WindowRefreshCallback(GLFWwindow* window);

		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		vec2 windowDPI = vec2(96.0f, 96.0f);
		BasePanel* m_resizingPanel = nullptr;
		ivec2 m_pendingResizeDelta = { 0, 0 };
		GLFWcursor* m_currentCursor = nullptr;
		GLFWcursor* m_arrowCursor = nullptr;
		GLFWcursor* m_hResizeCursor = nullptr;
		GLFWcursor* m_vResizeCursor = nullptr;

		int cleanupTextCounter = 0;

		static constexpr double DOUBLE_CLICK_TIME = 0.3;
		static constexpr int DOUBLE_CLICK_MAX_DIST = 5;

		std::array<double, SABLE_MAX_MOUSE_BUTTONS> m_lastClickTime = {};
		std::array<ivec2, SABLE_MAX_MOUSE_BUTTONS> m_lastClickPos = {};
	};
}