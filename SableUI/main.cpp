#include "SableUI/SableUI.h"

#define VSPLITTER SableUI::PanelType::VERTICAL
#define HSPLITTER SableUI::PanelType::HORIZONTAL
#define COMPONENT SableUI::PanelType::BASE
#define Colour SableUI::Colour
#define Color SableUI::Colour

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent() : SableUI::BaseComponent(Colour(80, 32, 32)) {};
};

int main(int argc, char** argv)
{
    using namespace SableUI;

    Window mainWindow("SableUI Layout Test", 1600, 1000);
    SetContext(&mainWindow);

    if (SableUI::SplitterPanel* rootHSplitter = SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL))
    {
        if (SableUI::SplitterPanel* vsplitter1 = SableUI::StartSplitter(SableUI::PanelType::VERTICAL))
        {
            if (SableUI::SplitterPanel* hsplitter1 = SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL))
            {
                if (SableUI::Panel* topLeftComponent = SableUI::AddPanel())
                {
                    topLeftComponent->AttachComponent<TestComponent>();
                }

                if (SableUI::SplitterPanel* vsplitter2 = SableUI::StartSplitter(SableUI::PanelType::VERTICAL))
                {
                    SableUI::Panel* nestedTopComponent = SableUI::AddPanel();
                    if (SableUI::SplitterPanel* hsplitter2 = SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL))
                    {
                        SableUI::Panel* nestedBottomLeftComponent = SableUI::AddPanel();
                        SableUI::Panel* nestedBottomRightComponent = SableUI::AddPanel();

                        SableUI::EndSplitter(); // HSplitter
                    }

                    SableUI::EndSplitter(); // VSplitter
                }

                SableUI::EndSplitter(); // TopHSplitter
            }
            SableUI::Panel* mainBottomComponent = SableUI::AddPanel();

            SableUI::EndSplitter(); // MainVSplitter
        }
        if (SableUI::SplitterPanel* rightVSplitter = SableUI::StartSplitter(SableUI::PanelType::VERTICAL))
        {
            SableUI::Panel* rightTopComponent = SableUI::AddPanel();
            SableUI::Panel* rightBottomComponent = SableUI::AddPanel();

            SableUI::EndSplitter(); // RightVSplitter
        }

        SableUI::EndSplitter(); // RootHSplitter
    }

    /* Can alternativley do (same thing)
    SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL);
    SableUI::StartSplitter(SableUI::PanelType::VERTICAL);
    SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL);
    SableUI::AddPanel();
    SableUI::StartSplitter(SableUI::PanelType::VERTICAL);
    SableUI::AddPanel();
    SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL);
    SableUI::AddPanel();
    SableUI::AddPanel();
    SableUI::EndSplitter();
    SableUI::EndSplitter();
    SableUI::EndSplitter();
    SableUI::AddPanel();
    SableUI::EndSplitter();
    SableUI::StartSplitter(SableUI::PanelType::VERTICAL);
    SableUI::AddPanel();
    SableUI::AddPanel();
    SableUI::EndSplitter();
    SableUI::EndSplitter(); */

    while (mainWindow.PollEvents())
    {
        mainWindow.Draw();
    }

    return 0;
}