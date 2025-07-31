#include "SableUI/SableUI.h"
#include <stack>

static SableUI::Window* currentContext = nullptr;
static SableUI::BasePanel* currentPanel = nullptr;
static std::stack<SableUI::BasePanel*> panelStack; // Stack to manage nested splitters

void SableUI::SetContext(SableUI::Window* window)
{
    currentContext = window;
    currentPanel = currentContext->GetRoot();
}

SableUI::SplitterPanel* SableUI::StartSplitter(PanelType orientation)
{
    if (currentContext == nullptr)
    {
        SableUI_Runtime_Error("No context set, call SetContext() first");
        return nullptr;
    }

    if (currentPanel == nullptr)
    {
        SableUI_Error("huh");
        return nullptr;
    }

    if (orientation != SableUI::PanelType::VERTICAL && orientation != SableUI::PanelType::HORIZONTAL)
    {
        SableUI_Error("Invalid panel type: %d, must be PanelType::VERTICAL or PanelType::HORIZONTAL", orientation);
        return nullptr;
    }

    SableUI::SplitterPanel* splitter = currentPanel->AddSplitter(orientation);
    panelStack.push(currentPanel);
    currentPanel = splitter;

    return splitter;
}

void SableUI::EndSplitter()
{
    if (panelStack.empty())
    {
        SableUI_Error("EndSplitter() called without a matching StartSplitter()");
        return;
    }

    currentPanel = panelStack.top();
    panelStack.pop();
}

SableUI::Panel* SableUI::AddPanel()
{
    if (currentContext == nullptr)
    {
        SableUI_Runtime_Error("No context set, call SetContext() first");
        return nullptr;
    }

    if (currentPanel == nullptr)
    {
        SableUI_Error("huh");
        return nullptr;
    }

    SableUI::Panel* panel = currentPanel->AddPanel();
    return panel;
}