#include <SableUI/SableUI.h>
#include <stack>
#include <thread>
#include <cstring>
#include <string.h>
#include <chrono>
#include <string>
#include <vector>
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

/* Panel builder */
static SableUI::Window* s_currentContext = nullptr;
static SableUI::BasePanel* s_currentPanel = nullptr;
static std::stack<SableUI::BasePanel*> s_panelStack;

static std::stack<SableUI::VirtualNode*> s_virtualStack;
static SableUI::VirtualNode* s_virtualRoot = nullptr;
static bool s_reconciliationMode = false;

using namespace SableMemory;

void SableUI::SetContext(SableUI::Window* window)
{
	s_currentContext = window;
	s_currentPanel = s_currentContext->GetRoot();
}

// ============================================================================
// Virtual Node Builder
// ============================================================================
void SableUI::StartDivVirtual(const SableUI::ElementInfo& info, SableUI::BaseComponent* child)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();

	auto* vnode = SB_new<VirtualNode>();
	vnode->type = ElementType::DIV;
	vnode->info = info;
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
	vnode->type = ElementType::RECT;
	vnode->info = info;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

void SableUI::AddTextVirtual(const std::string& text, const SableUI::ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->type = ElementType::TEXT;
	vnode->uniqueTextOrPath = text;
	vnode->info = info;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

void SableUI::AddTextU32Virtual(const SableString& text, const ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->type = ElementType::TEXT_U32;
	vnode->uniqueTextOrPath = text;
	vnode->info = info;

	if (parent) parent->children.push_back(vnode);
	else s_virtualRoot = vnode;
}

void SableUI::AddImageVirtual(const std::string& path, const SableUI::ElementInfo& info)
{
	VirtualNode* parent = s_virtualStack.empty() ? nullptr : s_virtualStack.top();
	auto* vnode = SB_new<VirtualNode>();
	vnode->type = ElementType::IMAGE;
	vnode->uniqueTextOrPath = path;
	vnode->info = info;

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
	return panel;
}

// ============================================================================
// Real Element Builder
// ============================================================================
static std::stack<SableUI::Element*> s_elementStack;
static SableUI::RendererBackend* s_elementRenderer = nullptr;

void SableUI::SetElementBuilderContext(RendererBackend* renderer, Element* rootElement, bool isVirtual)
{
	s_reconciliationMode = isVirtual;

	if (isVirtual)
	{
		SB_delete(s_virtualRoot);
		s_virtualStack = std::stack<VirtualNode*>();
		s_virtualRoot = SB_new<VirtualNode>();
		s_virtualRoot->info = rootElement->GetInfo();
		s_virtualRoot->type = rootElement->type;
		s_virtualRoot->uniqueTextOrPath = rootElement->uniqueTextOrPath;
		s_virtualStack.push(s_virtualRoot);
	}
	else
	{
		if (rootElement == nullptr)
		{
			SableUI_Error("rootElement is nullptr in SetElementBuilderContext");
			return;
		}

		s_elementRenderer = renderer;
		s_elementStack = std::stack<Element*>();
		s_elementStack.push(rootElement);
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

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

	if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();

	Element* newDiv = SB_new<Element>(s_elementRenderer, ElementType::DIV);
	newDiv->SetInfo(info);

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

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

	if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();
	Element* newRect = SB_new<Element>(s_elementRenderer, ElementType::RECT);

	newRect->SetInfo(info);
	parent->AddChild(newRect);
}

void SableUI::AddImage(const std::string& path, const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddImageVirtual(path, p_info);
	SableUI::ElementInfo info = p_info;

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

	if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();
	Element* newImage = SB_new<Element>(s_elementRenderer, ElementType::IMAGE);

	newImage->uniqueTextOrPath = path;
	newImage->SetInfo(info);
	newImage->SetImage(path);
	parent->AddChild(newImage);
}

void SableUI::AddText(const std::string& text, const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddTextVirtual(text, p_info);
	SableUI::ElementInfo info = p_info;

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;

	if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();
	Element* newText = SB_new<Element>(s_elementRenderer, ElementType::TEXT);

	newText->uniqueTextOrPath = text;
	newText->SetInfo(info);
	newText->SetText(text);
	parent->AddChild(newText);
}

void SableUI::AddTextU32(const SableString& text, const ElementInfo& p_info)
{
	if (s_reconciliationMode) return AddTextU32Virtual(text, p_info);
	SableUI::ElementInfo info = p_info;

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;

	if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return;
	}

	Element* parent = s_elementStack.top();
	Element* newTextU32 = SB_new<Element>(s_elementRenderer, ElementType::TEXT);

	newTextU32->uniqueTextOrPath = text;
	newTextU32->SetInfo(info);
	newTextU32->SetText(text);
	parent->AddChild(newTextU32);
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
	void Render();

	SableUI::Window* m_mainWindow = nullptr;
	std::vector<SableUI::Window*> m_secondaryWindows;

	std::chrono::milliseconds m_frameDuration = std::chrono::milliseconds(16);

private:
	using clock = std::chrono::high_resolution_clock;
	std::chrono::time_point<clock> m_nextFrameTime = clock::now();
};

static App* s_app = nullptr;
static SableUI::Backend s_backend = SableUI::Backend::UNDEF;

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

	if (s_backend != SableUI::Backend::UNDEF) return;

	s_backend = backend;
	SableUI_Log("Backend set to %s", s_backend == SableUI::Backend::OpenGL ? "OpenGL" : "Vulkan");
}

SableUI::Window* SableUI::Initialise(const char* name, int width, int height, int x, int y)
{
	if (s_app != nullptr)
	{
		SableUI_Runtime_Error("SableUI has already been initialised");
	}

	if (s_backend == SableUI::Backend::UNDEF)
	{
		s_backend = SableUI::Backend::OpenGL;
	}

	s_app = SB_new<App>(name, width, height, x, y);

	return s_app->m_mainWindow;
}

void SableUI::SetMaxFPS(int fps)
{
	if (s_app == nullptr)
	{
		SableUI_Runtime_Error("SableUI has not been initialised");
		return;
	}

	s_app->m_frameDuration = std::chrono::milliseconds(1000 / fps);
}

void SableUI::Shutdown()
{
	if (s_app == nullptr)
	{
		SableUI_Runtime_Error("SableUI has not been initialised");
	}

	SB_delete(s_app);
	s_app = nullptr;
	
	SableMemory::DestroyPools();
	SableUI_Log("Shut down successfully");
}

SableUI::Window* SableUI::CreateSecondaryWindow(const char* name, int width, int height, int x, int y)
{
	if (s_app == nullptr)
	{
		SableUI_Runtime_Error("SableUI has not been initialised");
	}

	return s_app->CreateSecondaryWindow(name, width, height, x, y);
}

bool SableUI::PollEvents() {
	if (s_app == nullptr) return false;

	SableMemory::CompactPools();

	return s_app->PollEvents();
}

void SableUI::Render() {
	if (s_app == nullptr) return;
	s_app->Render();
}

App::App(const char* name, int width, int height, int x, int y)
{
	SableMemory::InitPools();
	SableUI::SableUI_Window_Initalise_GLFW();

	m_mainWindow = SB_new<SableUI::Window>(s_backend, nullptr, name, width, height, x, y);

	SableUI::InitFontManager();
	SableUI::InitDrawables();
	SableUI::SetContext(m_mainWindow);
}

SableUI::Window* App::CreateSecondaryWindow(const char* name, int width, int height, int x, int y)
{
	if (m_mainWindow == nullptr) SableUI_Runtime_Error("Cannot create secondary window without main window");

	SableUI::Window* window = SB_new<SableUI::Window>(s_backend, s_app->m_mainWindow, name, width, height, x, y);
	m_secondaryWindows.push_back(window);
	SableUI::SetContext(window);
	return window;
}

bool App::PollEvents()
{
	if (m_mainWindow == nullptr) return false;
	if (!m_mainWindow->PollEvents()) return false;

	for (auto it = m_secondaryWindows.begin(); it != m_secondaryWindows.end();)
	{
		SableUI::Window* window = *it;
		if (!window->PollEvents())
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

	auto now = clock::now();

	if (now < m_nextFrameTime)
		std::this_thread::sleep_until(m_nextFrameTime);

	m_nextFrameTime = clock::now() + m_frameDuration;

	m_mainWindow->AddToDrawStack();

	for (SableUI::Window* window : m_secondaryWindows)
		window->AddToDrawStack();
}

App::~App()
{
	SableUI::DestroyFontManager();
	SableUI::DestroyDrawables();

	for (SableUI::Window* window : m_secondaryWindows) SB_delete(window);
	m_secondaryWindows.clear();

	SB_delete(m_mainWindow);
	m_mainWindow = nullptr;

	SableUI::SableUI_Window_Terminate_GLFW();
}