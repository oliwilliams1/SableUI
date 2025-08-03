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
    s_elementRenderer = renderer;
    s_elementStack = {};
    if (rootElement)
    {
        s_elementStack.push(rootElement);
    }
}

SableUI::Element* SableUI::StartDiv(const ElementInfo& p_info)
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
    parent->AddChild(newDiv);
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

SableUI::Element* SableUI::AddText(const std::u32string& text, const ElementInfo& info)
{
    return nullptr;
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

    if (info.wType == RectType::UNDEF) info.wType = RectType::FIT_CONTENT;
    if (info.hType == RectType::UNDEF) info.hType = RectType::FIT_CONTENT;

    if (s_elementStack.empty() || s_elementRenderer == nullptr)
    {
        SableUI_Error("Element context not set. Call SetElementBuilderContext() first");
        return nullptr;
    }

    Element* parent = s_elementStack.top();
    Element* newText = new Element(s_elementRenderer, ElementType::TEXT);

    newText->SetInfo(info);
    newText->SetText(std::u32string(text.begin(), text.end()));
    parent->AddChild(newText);

    return newText;
}

SableUI::Element* SableUI::AddTextU32(const std::u32string& text, const ElementInfo& p_info)
{
    SableUI::ElementInfo info = p_info;

    if (info.wType == RectType::UNDEF) info.wType = RectType::FIT_CONTENT;
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
