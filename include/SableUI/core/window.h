#pragma once
#include <SableUI/core/renderer.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/floating_panel.h>
#include <SableUI/core/drawable.h>

#include <unordered_map>
#include <string>
#include <array>
#include <vector>
#include <utility>

struct GLFWcursor;
struct GLFWwindow;

namespace SableUI
{
	void SableUI_Window_Initialise_GLFW();
	void SableUI_Window_Terminate_GLFW();
	void SableUI_Window_PollEvents_GLFW();
	void SableUI_Window_WaitEvents_GLFW();
	void SableUI_Window_PostEmptyEvent_GLFW();
	void SableUI_Window_WaitEventsTimeout_GLFW(double timeout);
	void* GetCurrentContext_voidType();

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

	struct WindowInitInfo
	{
		int posX = -1;
		int posY = -1;
		bool decorated = true;
		bool resisable = true;
		bool floating = false;
		bool maximised = false;
	};

	class Window
	{
	public:
		Window(const Backend& backend, Window* primary, const std::string& title, int width, int height, const WindowInitInfo& info);
		~Window();

		bool Update(const std::unordered_set<TimerHandle>& firedTimers);
		void Draw();
		bool m_needsStaticRedraw = false;
		bool m_needsRefresh = true;
		void SetTitleBar(const SableString& title);

		RootPanel* GetRoot();

		void RerenderAllNodes();
		void RecalculateNodes();

		SableString GetClipboardContent();
		void SetClipboardContent(const SableString& content);

		UIEventContext ctx;
		ivec2 m_windowSize = { 0, 0 };

		void SubmitCustomQueue(CustomTargetQueue* queue);
		void RemoveQueueReference(CustomTargetQueue* queue);
		GpuFramebuffer* GetSurface() { return &m_windowSurface; }
		RendererBackend* GetBaseRenderer() const { return m_baseRenderer; }

		void CreateFloatingPanel(const std::string& id, const std::string& componentName, const Rect& r);
		void QueueDestroyFloatingPanel(const std::string& id);
		bool IsFloatingPanelActive(const std::string& id) const;

		template <typename T>
		T* CreateFloatingPanelNoInit(const std::string& id, const Rect& r);

		void MakeContextCurrent();
		bool IsMinimized() const;

		void ExecuteDestroyFloatingPanelQueue();
	
	private:
		GpuFramebuffer m_baseFramebuffer;
		RendererBackend* m_baseRenderer = nullptr;
		GpuTexture2D m_baseColourAttachment;

		RendererBackend* m_floatingRenderer = nullptr;
		GpuFramebuffer m_floatingFramebuffer;
		GpuTexture2D m_floatingColourAttachment;

		GpuFramebuffer m_windowSurface;

		void HandleResize();
		GLFWcursor* CheckResize(BasePanel* node, bool* resCalled, bool isLastChild);
		void Resize(ivec2 pos, BasePanel* panel = nullptr);
		void ResizeStep(ivec2 deltaPos, BasePanel* panel, BasePanel* root);
		ResizeState m_resizeState;

		RootPanel* m_root = nullptr;
		bool m_resizing = false;
		bool m_isMinimized = false;

		static void MousePosCallback(GLFWwindow* window, double x, double y);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void ResizeCallback(GLFWwindow* window, int width, int height);
		static void WindowRefreshCallback(GLFWwindow* window);
		static void ScrollCallback(GLFWwindow* window, double x, double y);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void CharCallback(GLFWwindow* window, unsigned int codepoint);

		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		vec2 windowDPI = vec2(96.0f, 96.0f);
		BasePanel* m_resizingPanel = nullptr;
		ivec2 m_pendingResizeDelta = { 0, 0 };
		GLFWcursor* m_currentCursor = nullptr;
		GLFWcursor* m_arrowCursor = nullptr;
		GLFWcursor* m_hResizeCursor = nullptr;
		GLFWcursor* m_vResizeCursor = nullptr;

		static constexpr double DOUBLE_CLICK_TIME = 0.3;
		static constexpr int DOUBLE_CLICK_MAX_DIST = 5;

		std::array<double, SABLE_MAX_MOUSE_BUTTONS> m_lastClickTime = {};
		std::array<ivec2, SABLE_MAX_MOUSE_BUTTONS> m_lastClickPos = {};

		std::vector<CustomTargetQueue*> m_customTargetQueues;
		void DestroyFloatingPanel(const std::string& id);
		std::unordered_map<std::string, FloatingPanel*> m_floatingPanels;
		std::vector<std::string> m_destroyPanelQueue;

	private:
		DrawableRect* m_borderTop = nullptr;
		DrawableRect* m_borderBottom = nullptr;
		DrawableRect* m_borderLeft = nullptr;
		DrawableRect* m_borderRight = nullptr;
		bool m_borderNeedsUpdate = true;
		int m_syncFrames = 2;

		void UpdateWindowBorder();
		void RenderWindowBorder();
	};

	class BaseComponent;
	template <typename T>
	inline T* Window::CreateFloatingPanelNoInit(const std::string& id, const Rect& r)
	{
		auto it = m_floatingPanels.find(id);
		if (it != m_floatingPanels.end())
		{
			SableUI_Runtime_Error("CreateFloatingPanel() called with an already existing ID");
			return nullptr;
		}

		FloatingPanel* newPanel = SableMemory::SB_new<FloatingPanel>(m_floatingRenderer, r);
		T* comp = newPanel->AttachComponentByType<T>();
		newPanel->SetComponent(comp);
		m_floatingPanels[id] = newPanel;

		return comp;
	}
}
