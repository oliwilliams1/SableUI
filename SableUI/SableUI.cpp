#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <SableUI/memory.h>
#include <SableUI/console.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h>
#include <SableUI/panel.h>
#include <SableUI/renderer.h>
#include <SableUI/text.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
#include <SableUI/componentRegistry.h>
#include <SableUI/worker_pool.h>
#include <stack>
#include <cstring>
#include <string.h>
#include <string>
#include <vector>

/* Panel builder */
static SableUI::Window* s_currentContext = nullptr;
static SableUI::BasePanel* s_currentPanel = nullptr;
static std::stack<SableUI::BasePanel*> s_panelStack;

static std::stack<SableUI::VirtualNode*> s_virtualStack;
static SableUI::VirtualNode* s_virtualRoot = nullptr;
static bool s_reconciliationMode = false;

static std::stack<SableUI::Element*> s_elementStack;
static std::stack<SableUI::RendererBackend*> s_rendererStack;
static SableUI::BaseComponent* s_currentComponent = nullptr;

using namespace SableMemory;

static void SetContext(SableUI::Window* window)
{
	s_currentContext = window;
	s_currentPanel = s_currentContext->GetRoot();
}

static SableUI::ivec2 g_nextPanelMaxBounds = { 0, 0 };
static SableUI::ivec2 g_nextPanelMinBounds = { 0, 0 };

void SableUI::SetNextPanelMaxHeight(int height)	  { g_nextPanelMaxBounds.h = height; }
void SableUI::SetNextPanelMaxWidth(int width)	  { g_nextPanelMaxBounds.w = width; };
void SableUI::SetNextPanelMinBounds(ivec2 bounds) { g_nextPanelMinBounds = bounds; }

// ============================================================================
// Virtual Node Builder
// ============================================================================
void SableUI::StartDivVirtual(const SableUI::ElementInfo& info, SableUI::BaseComponent* child)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();

	auto* vnode = SB_new<VirtualNode>();
	vnode->info = info;
	vnode->info.type = ElementType::Div;
	vnode->childComp = child;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;

	s_virtualStack.push(vnode);
}

void SableUI::EndDivVirtual()
{
	if (s_virtualStack.empty()) return;
	s_virtualStack.pop();
}

void SableUI::AddRectVirtual(const SableUI::ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->info = info;
	vnode->info.type = ElementType::Rect;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

void SableUI::AddTextVirtual(const SableString& text, const SableUI::ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->info = info;
	vnode->info.type = ElementType::Text;
	vnode->info.text.content = text;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

void SableUI::AddImageVirtual(const SableString& path, const SableUI::ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->info = info;
	vnode->info.type = ElementType::Image;
	vnode->info.text.content = path;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

// ============================================================================
// Splitters & Panels
// ============================================================================
SableUI::SplitterPanel* SableUI::StartSplitter(PanelType orientation)
{
	if (s_currentContext == nullptr)
	{
		SableUI_Runtime_Error("No context set, call SetContext() first");
		return nullptr;
	}

	if (s_currentPanel == nullptr)
	{
		SableUI_Error("Current panel is null, this should not happen");
		return nullptr;
	}

	if (orientation != SableUI::PanelType::VERTICAL && orientation != SableUI::PanelType::HORIZONTAL)
	{
		SableUI_Error("Invalid panel type: %d, must be PanelType::VERTICAL or PanelType::HORIZONTAL", static_cast<int>(orientation));
		return nullptr;
	}

	SableUI::SplitterPanel* splitter = s_currentPanel->AddSplitter(orientation);
	splitter->maxBounds = g_nextPanelMaxBounds;
	g_nextPanelMaxBounds = { 0, 0 };

	if (g_nextPanelMinBounds.x > 20 || g_nextPanelMinBounds.y > 20)
	{
		SableUI_Runtime_Error("You cannot sent min bounds of splitters");
	}

	s_panelStack.push(s_currentPanel);
	s_currentPanel = splitter;

	return splitter;
}

void SableUI::EndSplitter()
{
	if (s_panelStack.empty())
	{
		SableUI_Error("EndSplitter() called without a matching StartSplitter()");
		return;
	}

	s_currentPanel = s_panelStack.top();
	s_panelStack.pop();
}

SableUI::ContentPanel* SableUI::AddPanel()
{
	if (s_currentContext == nullptr)
	{
		SableUI_Runtime_Error("No context set, call SetContext() first");
		return nullptr;
	}

	if (s_currentPanel == nullptr)
	{
		SableUI_Error("Current panel is null, this should not happen");
		return nullptr;
	}

	SableUI::ContentPanel* panel = s_currentPanel->AddPanel();
	panel->maxBounds = g_nextPanelMaxBounds;
	panel->minBounds = g_nextPanelMinBounds;
	g_nextPanelMinBounds = { 20, 20 };
	g_nextPanelMaxBounds = { 0, 0 };
	return panel;
}

// ============================================================================
// Real Element Builder
// ============================================================================
void SableUI::SetCurrentComponent(BaseComponent* component)
{
	s_currentComponent = component;
}

void SableUI::SetElementBuilderContext(RendererBackend* renderer, Element* rootElement, bool isVirtual)
{
	s_reconciliationMode = isVirtual;

	if (isVirtual)
	{
		SB_delete(s_virtualRoot);
		s_virtualStack = std::stack<VirtualNode*>();
		s_virtualRoot = SB_new<VirtualNode>();
		s_virtualRoot->info = rootElement->GetInfo();
		s_virtualStack.push(s_virtualRoot);
	}
	else
	{
		if (rootElement == nullptr)
		{
			SableUI_Error("rootElement is nullptr in SetElementBuilderContext");
			return;
		}

		s_rendererStack = std::stack<RendererBackend*>();
		s_rendererStack.push(renderer);
		s_elementStack = std::stack<Element*>();
		s_elementStack.push(rootElement);

		if (s_currentComponent)
		{
			rootElement->m_owner = s_currentComponent;
		}
	}
}

SableUI::VirtualNode* SableUI::GetVirtualRootNode()
{
	return s_virtualRoot;
}

SableUI::Element* SableUI::GetCurrentElement()
{
	if (s_elementStack.empty())
	{
		SableUI_Error("GetCurrentElement() called without a matching SetElementBuilderContext()");
		return nullptr;
	}

	return s_elementStack.top();
}

void SableUI::StartDiv(const ElementInfo& p_info, BaseComponent* child)
{
	if (s_reconciliationMode) return StartDivVirtual(p_info, child);
	SableUI::ElementInfo info = p_info;

	if (info.appearance.inheritBg && info.appearance.bg == Colour{ 0, 0, 0, 0 })
		info.appearance.bg = s_elementStack.top()->info.appearance.bg;

	if (info.layout.wType == RectType::Undef) info.layout.wType = RectType::FitContent;
	if (info.layout.hType == RectType::Undef) info.layout.hType = RectType::FitContent;

	if (s_elementStack.empty() || s_rendererStack.top() == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();

	info.type = ElementType::Div;
	Element* newDiv = SB_new<Element>(s_rendererStack.top(), info);

	newDiv->m_owner = parent->m_owner;
	newDiv->RegisterForHover();

	if (child == nullptr)
	{
		parent->AddChild(newDiv);
	}
	else
	{
		child->SetRootElement(newDiv);
		parent->AddChild(SB_new<Child>(child));
	}

	s_elementStack.push(newDiv);
}

void SableUI::EndDiv()
{
	if (s_reconciliationMode) return EndDivVirtual();
	if (s_elementStack.empty())
	{
		SableUI_Error("EndDiv() called without a matching StartDiv()");
		return;
	}

	s_elementStack.pop();
}

void SableUI::AddRect(const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddRectVirtual(p_info);
	SableUI::ElementInfo info = p_info;

	if (info.appearance.inheritBg && info.appearance.bg == Colour{ 0, 0, 0, 0 })
		info.appearance.bg = s_elementStack.top()->info.appearance.bg;

	if (info.layout.wType == RectType::Undef) info.layout.wType = RectType::FitContent;
	if (info.layout.hType == RectType::Undef) info.layout.hType = RectType::FitContent;

	if (s_elementStack.empty() || s_rendererStack.top() == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();

	info.type = ElementType::Rect;
	Element* newRect = SB_new<Element>(s_rendererStack.top(), info);

	newRect->m_owner = parent->m_owner;
	newRect->RegisterForHover();

	parent->AddChild(newRect);
}

void SableUI::AddImage(const SableString& path, const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddImageVirtual(path, p_info);
	SableUI::ElementInfo info = p_info;

	if (info.appearance.inheritBg && info.appearance.bg == Colour{ 0, 0, 0, 0 })
		info.appearance.bg = s_elementStack.top()->info.appearance.bg;

	if (info.layout.wType == RectType::Undef) info.layout.wType = RectType::FitContent;
	if (info.layout.hType == RectType::Undef) info.layout.hType = RectType::FitContent;

	if (s_elementStack.empty() || s_rendererStack.top() == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();

	info.type = ElementType::Image;
	Element* newImage = SB_new<Element>(s_rendererStack.top(), info);

	newImage->m_owner = parent->m_owner;
	newImage->RegisterForHover();

	newImage->SetImage(path);
	parent->AddChild(newImage);
}

void SableUI::AddText(const SableString& text, const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddTextVirtual(text, p_info);
	SableUI::ElementInfo info = p_info;

	if (info.appearance.inheritBg && info.appearance.bg == Colour{ 0, 0, 0, 0 })
		info.appearance.bg = s_elementStack.top()->info.appearance.bg;

	if (info.layout.wType == RectType::Undef) info.layout.wType = RectType::Fill;
	if (info.layout.hType == RectType::Undef) info.layout.hType = RectType::FitContent;

	if (s_elementStack.empty() || s_rendererStack.top() == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();

	info.type = ElementType::Text;
	Element* newTextU32 = SB_new<Element>(s_rendererStack.top(), info);

	newTextU32->m_owner = parent->m_owner;
	newTextU32->RegisterForHover();

	newTextU32->SetText(text);
	parent->AddChild(newTextU32);
}

// ============================================================================
// CustomLayoutTarget Element Builder
// ============================================================================
bool s_customLayoutMode = false;
void SableUI::StartCustomLayoutScope(
	CustomTargetQueue* queuePtr,
	const ElementInfo& elementInfo)
{
	if (s_customLayoutMode)
		SableUI_Runtime_Error("Cannot nest custom layouts");

	if (s_reconciliationMode)
		SableUI_Runtime_Error("Custom layouts not supported in reconciliation yet");

	if (!queuePtr->window)
	{
		SableUI_Runtime_Error("Custom target queue does not have a context");
		return;
	}

	if (!queuePtr->target)
		SableUI_Runtime_Error("Custom target queue does not have a target");

	if (queuePtr != nullptr)
	{
		queuePtr->window->RemoveQueueReference(queuePtr);
		if (queuePtr->root)
		{
			SB_delete(queuePtr->root);
			queuePtr->root = nullptr;

			for (DrawableBase* dr : queuePtr->drawables)
				SB_delete(dr);
			
			queuePtr->drawables.clear();
		}
	}
	else
	{
		SableUI_Runtime_Error("Custom target queue not initialised");
		return;
	}

	s_rendererStack.push(queuePtr->window->m_renderer);
	ElementInfo info = elementInfo;
	info.type = ElementType::Div;
	info.layout.wType = RectType::FitContent;
	info.layout.hType = RectType::FitContent;
	Element* queueRoot = SB_new<Element>(queuePtr->window->m_renderer, info);

	queuePtr->root = queueRoot;
	s_elementStack.push(queueRoot);
	s_customLayoutMode = true;
}

void SableUI::EndCustomLayoutScope(
	CustomTargetQueue* queuePtr)
{
	if (!s_customLayoutMode)
		SableUI_Runtime_Error("EndCustomLayoutScope called without StartCustomLayoutScope");

	if (!queuePtr)
	{
		SableUI_Runtime_Error("Custom target queue not initialised");
		return;
	}

	if (!queuePtr->window)
	{
		SableUI_Runtime_Error("Custom target queue does not have a context");
		return;
	}

	queuePtr->window->SubmitCustomQueue(queuePtr);
	s_elementStack.top()->LayoutChildren();
	s_rendererStack.pop();
	s_elementStack.pop();

	s_customLayoutMode = false;
}

// ============================================================================
// App
// ============================================================================
class App
{
public:
	App(const char* name = "SableUI", int width = 800, int height = 600, int x = -1, int y = -1);
	~App();

	SableUI::Window* CreateSecondaryWindow(const char* name, int width, int height, int x, int y);

	bool PollEvents();
	bool WaitEvents();
	bool WaitEventsTimeout(double timeout);
	void Render();

	SableUI::Window* m_mainWindow = nullptr;
	std::vector<SableUI::Window*> m_secondaryWindows;
};

static App* s_app = nullptr;
static SableUI::Backend s_backend = SableUI::Backend::Undef;

void SableUI::PreInit(int argc, char** argv)
{
	if (s_app != nullptr)
	{
		SableUI_Runtime_Error("SableUI::PreInit() must be called before SableUI::Initalise()");
	}

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--opengl") == 0)
		{
			s_backend = SableUI::Backend::OpenGL;
			return;
		}
		else if (strcmp(argv[i], "--vulkan") == 0)
		{
			s_backend = SableUI::Backend::Vulkan;
			return;
		}
	}
}

void SableUI::SetBackend(const SableUI::Backend& backend)
{
	if (s_app != nullptr)
	{
		SableUI_Error("SableUI::SetBackend() must be called before SableUI::Initalise()");
		return;
	}

	if (s_backend != SableUI::Backend::Undef) return;

	s_backend = backend;
	SableUI_Log("Backend set to %s", s_backend == SableUI::Backend::OpenGL ? "OpenGL" : "Vulkan");
}
 
SableUI::Window* SableUI::Initialise(const char* name, int width, int height, int x, int y)
{
	if (s_app != nullptr)
	{
		SableUI_Runtime_Error("SableUI has already been initialised");
	}

	RegisterSableUIComponents();

	if (s_backend == SableUI::Backend::Undef)
	{
		s_backend = SableUI::Backend::OpenGL;
	}

	WorkerPool::Initialise();

	s_app = SB_new<App>(name, width, height, x, y);

	return s_app->m_mainWindow;
}

void SableUI::Shutdown()
{
	if (s_app == nullptr)
	{
		SableUI_Runtime_Error("SableUI has not been initialised");
	}

	SB_delete(s_app);
	s_app = nullptr;

	WorkerPool::Shutdown();
	
	SableMemory::DestroyPools();
	SableUI_Log("Shut down successfully");
}

SableUI::Window* SableUI::CreateSecondaryWindow(const char* name, int width, int height, int x, int y)
{
	if (s_app == nullptr)
		SableUI_Runtime_Error("SableUI has not been initialised");

	return s_app->CreateSecondaryWindow(name, width, height, x, y);
}

bool SableUI::PollEvents()
{
	if (s_app == nullptr) return false;

	SableMemory::CompactPools();

	return s_app->PollEvents();
}

bool SableUI::WaitEvents()
{
	if (s_app == nullptr) return false;

	SableMemory::CompactPools();

	return s_app->WaitEvents();
}

bool SableUI::WaitEventsTimeout(double timeout)
{
	if (s_app == nullptr) return false;

	SableMemory::CompactPools();

	return s_app->WaitEventsTimeout(timeout);
}

void SableUI::Render()
{
	if (s_app == nullptr) return;
	s_app->Render();
}

static bool s_eventPostedThisFrame = false;
void SableUI::PostEmptyEvent()
{
	if (!s_eventPostedThisFrame)
	{
		SableUI_Window_PostEmptyEvent_GLFW();
		s_eventPostedThisFrame = true;
	}
}

App::App(const char* name, int width, int height, int x, int y)
{
	SableMemory::InitPools();
	SableUI::SableUI_Window_Initalise_GLFW();

	m_mainWindow = SB_new<SableUI::Window>(s_backend, nullptr, name, width, height, x, y);

	SableUI::InitFontManager();
	SableUI::InitDrawables();
	SetContext(m_mainWindow);
}

SableUI::Window* App::CreateSecondaryWindow(const char* name, int width, int height, int x, int y)
{
	if (m_mainWindow == nullptr) SableUI_Runtime_Error("Cannot create secondary window without main window");

	SableUI::Window* window = SB_new<SableUI::Window>(s_backend, s_app->m_mainWindow, name, width, height, x, y);
	m_secondaryWindows.push_back(window);
	SetContext(window);
	return window;
}

bool App::PollEvents()
{
	s_eventPostedThisFrame = false;
	if (m_mainWindow == nullptr) return false;
	SableUI::SableUI_Window_PollEvents_GLFW();

	if (!m_mainWindow->Update()) return false;

	for (auto it = m_secondaryWindows.begin(); it != m_secondaryWindows.end();)
	{
		SableUI::Window* window = *it;
		if (!window->Update())
		{
			SB_delete(window);
			it = m_secondaryWindows.erase(it);
		}
		else
		{
			it++;
		}
	}

	return true;
}

bool App::WaitEvents()
{
	s_eventPostedThisFrame = false;
	if (m_mainWindow == nullptr) return false;
	SableUI::SableUI_Window_WaitEvents_GLFW();

	if (!m_mainWindow->Update()) return false;

	for (auto it = m_secondaryWindows.begin(); it != m_secondaryWindows.end();)
	{
		SableUI::Window* window = *it;
		if (!window->Update())
		{
			SB_delete(window);
			it = m_secondaryWindows.erase(it);
		}
		else
		{
			it++;
		}
	}

	return true;
}

bool App::WaitEventsTimeout(double timeout)
{
	s_eventPostedThisFrame = false;
	if (m_mainWindow == nullptr) return false;
	SableUI::SableUI_Window_WaitEventsTimeout_GLFW(timeout);

	if (!m_mainWindow->Update()) return false;

	for (auto it = m_secondaryWindows.begin(); it != m_secondaryWindows.end();)
	{
		SableUI::Window* window = *it;
		if (!window->Update())
		{
			SB_delete(window);
			it = m_secondaryWindows.erase(it);
		}
		else
		{
			it++;
		}
	}

	return true;
}

void App::Render()
{
	if (m_mainWindow == nullptr)
		return;

	m_mainWindow->Draw();

	for (SableUI::Window* window : m_secondaryWindows)
		window->Draw();
}

App::~App()
{
	SableUI::DestroyFontManager();
	SableUI::DestroyDrawables();

	for (SableUI::Window* window : m_secondaryWindows) SB_delete(window);
	m_secondaryWindows.clear();

	SB_delete(m_mainWindow);
	m_mainWindow = nullptr;

	SableUI::ComponentRegistry::Shutdown();
	SableUI::SableUI_Window_Terminate_GLFW();
}