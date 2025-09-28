#pragma once
#pragma warning(push)
#pragma warning(disable : 4005)
#include <string>
#include <chrono>

#include "SableUI/panel.h"
#pragma warning(pop)

struct GLFWcursor;
struct GLFWwindow;

namespace SableUI
{
	void SableUI_Window_Initalise_GLFW();
	void SableUI_Window_Terminate_GLFW();

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

	private:
		void InitOpenGL();
		void InitVulkan();

		int GetRefreshRate();
		Renderer m_renderer;

		void Resize(ivec2 pos, BasePanel* node = nullptr);
		GLFWcursor* CheckResize(BasePanel* node, bool* resCalled);

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