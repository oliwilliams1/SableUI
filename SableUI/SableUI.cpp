#include "SableUI/SableUI.h"
#include <stack>

/* Panel builder */
static SableUI::Window* s_currentContext = nullptr;
static SableUI::BasePanel* s_currentPanel = nullptr;
static std::stack<SableUI::BasePanel*> s_panelStack;

void SableUI::SetContext(SableUI::Window* window)
{
    s_currentContext = window;
    s_currentPanel = s_currentContext->GetRoot();
}

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

SableUI::Panel* SableUI::AddPanel()
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

    SableUI::Panel* panel = s_currentPanel->AddPanel();
    return panel;
}

/* Element builder */
static std::stack<SableUI::Element*> s_elementStack;
static SableUI::Renderer* s_elementRenderer = nullptr;

void SableUI::SetElementBuilderContext(Renderer* renderer, Element* rootElement)
{
    if (rootElement == nullptr)
    {
        SableUI_Error("rootElement is nullptr in SetElementBuilderContext");
    }

    s_elementRenderer = renderer;
    s_elementStack = {};
    if (rootElement)
    {
        s_elementStack.push(rootElement);
    }
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

SableUI::Element* SableUI::StartDiv(const ElementInfo& p_info, BaseComponent* child)
{
    SableUI::ElementInfo info = p_info;

    if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
    if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
    {
        SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
        return nullptr;
    }

    Element* parent = s_elementStack.top();

    Element* newDiv = new Element(s_elementRenderer, ElementType::DIV);
    newDiv->SetInfo(info);

    if (child == nullptr)
    {
        parent->AddChild(newDiv);
    }
    else
    {
        child->SetRootElement(newDiv);
        parent->AddChild(new Child(child));
    }

    s_elementStack.push(newDiv);
    return newDiv;
}

void SableUI::EndDiv()
{
    if (s_elementStack.empty())
    {
        SableUI_Error("EndDiv() called without a matching StartDiv()");
        return;
    }

    s_elementStack.pop();
}

SableUI::Element* SableUI::AddRect(const ElementInfo& p_info)
{
    SableUI::ElementInfo info = p_info;

    if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
    if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
    {
        SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
        return nullptr;
    }

    Element* parent = s_elementStack.top();
    Element* newRect = new Element(s_elementRenderer, ElementType::RECT);

    newRect->SetInfo(info);
    parent->AddChild(newRect);

    return newRect;
}

SableUI::Element* SableUI::AddImage(const std::string& path, const ElementInfo& p_info)
{
    SableUI::ElementInfo info = p_info;

	if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
	if (info.hType == RectType::UNDEF) info.hType = RectType::FILL;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
	{
		SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
		return nullptr;
	}

    Element* parent = s_elementStack.top();
    Element* newImage = new Element(s_elementRenderer, ElementType::IMAGE);

    newImage->SetInfo(info);
	newImage->SetImage(path);
	parent->AddChild(newImage);

    return newImage;
}

SableUI::Element* SableUI::AddText(const std::string& text, const ElementInfo& p_info)
{
    SableUI::ElementInfo info = p_info;

    if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
    if (info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
    {
        SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
        return nullptr;
    }

    Element* parent = s_elementStack.top();
    Element* newText = new Element(s_elementRenderer, ElementType::TEXT);

    newText->SetInfo(info);
    newText->SetText(text.c_str());
    parent->AddChild(newText);

    return newText;
}

SableUI::Element* SableUI::AddTextU32(const SableString& text, const ElementInfo& p_info)
{
    SableUI::ElementInfo info = p_info;

    if (info.wType == RectType::UNDEF) info.wType = RectType::FILL;
    if (info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
    {
        SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
        return nullptr;
    }

    Element* parent = s_elementStack.top();
    Element* newTextU32 = new Element(s_elementRenderer, ElementType::TEXT);

    newTextU32->SetInfo(info);
    newTextU32->SetText(text);
    parent->AddChild(newTextU32);

    return newTextU32;
}

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

    s_app = new App(name, width, height, x, y);

    return s_app->m_mainWindow;
}

void SableUI::Shutdown()
{
    if (s_app == nullptr)
    {
        SableUI_Runtime_Error("SableUI has not been initialised");
    }

    delete s_app;
    s_app = nullptr;
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
    return s_app->PollEvents();
}

void SableUI::Render() {
    if (s_app == nullptr) return;
    s_app->Render();
}

/* ------------- APP IMPLEMENTATION ------------- */

App::App(const char* name, int width, int height, int x, int y)
{
    SableUI::SableUI_Window_Initalise_GLFW();

    m_mainWindow = new SableUI::Window(s_backend, nullptr, name, width, height, x, y);

    SableUI::InitFontManager();
    SableUI::InitDrawables();
    SableUI::SetContext(m_mainWindow);
}

SableUI::Window* App::CreateSecondaryWindow(const char* name, int width, int height, int x, int y)
{
    if (m_mainWindow == nullptr) SableUI_Runtime_Error("Cannot create secondary window without main window");

    SableUI::Window* window = new SableUI::Window(s_backend, s_app->m_mainWindow, name, width, height, x, y);
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
            delete window;
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
    if (m_mainWindow == nullptr) return;

    m_mainWindow->Draw();

    for (SableUI::Window* window : m_secondaryWindows)
    {
        window->Draw();
    }
}

App::~App()
{
    SableUI::DestroyFontManager();
    SableUI::DestroyDrawables();

    for (SableUI::Window* window : m_secondaryWindows) delete window;
    m_secondaryWindows.clear();

    delete m_mainWindow;
    m_mainWindow = nullptr;

    SableUI::SableUI_Window_Terminate_GLFW();

    SableUI_Log("Shut down successfully");
}