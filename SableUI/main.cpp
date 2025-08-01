#include "SableUI/SableUI.h"

#define VERTICAL SableUI::PanelType::VERTICAL
#define HORIZONTAL SableUI::PanelType::HORIZONTAL
#define Colour SableUI::Colour
#define Color SableUI::Colour

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent(int r) : SableUI::BaseComponent(Colour(r, 32, 32)) {};

    void Init() override
    {
        SableUI::StartRect(SableUI::ElementInfo{}.setWType(SableUI::RectType::FIXED).setWidth(128).setHType(SableUI::RectType::FIXED).setHeight(128));
        SableUI::EndRect();
    }
};

int main(int argc, char** argv)
{
    using namespace SableUI;

    Window mainWindow("SableUI Layout Test", 1600, 1000);
    SetContext(&mainWindow);

    if (SableUI::SplitterPanel* rootHSplitter = SableUI::StartSplitter(HORIZONTAL))
    {
        if (SableUI::SplitterPanel* vsplitter1 = SableUI::StartSplitter(VERTICAL))
        {
            if (SableUI::SplitterPanel* hsplitter1 = SableUI::StartSplitter(HORIZONTAL))
            {
                if (SableUI::Panel* topLeftComponent = SableUI::AddPanel())
                {
                    topLeftComponent->AttachComponent<TestComponent>(128);
                }

                if (SableUI::SplitterPanel* vsplitter2 = SableUI::StartSplitter(VERTICAL))
                {
                    SableUI::Panel* nestedTopComponent = SableUI::AddPanel();
                    if (SableUI::SplitterPanel* hsplitter2 = SableUI::StartSplitter(HORIZONTAL))
                    {
                        if (SableUI::Panel* nestedBottomLeftComponent = SableUI::AddPanel())
                        {
                            nestedBottomLeftComponent->AttachComponent<TestComponent>(80);
                        }
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
        if (SableUI::SplitterPanel* rightVSplitter = SableUI::StartSplitter(VERTICAL))
        {
            SableUI::Panel* rightTopComponent = SableUI::AddPanel();
            SableUI::Panel* rightBottomComponent = SableUI::AddPanel();

            SableUI::EndSplitter(); // RightVSplitter
        }

        SableUI::EndSplitter(); // RootHSplitter
    }

    /* Can alternativley do (same thing)
    SableUI::StartSplitter(HORIZONTAL);
    SableUI::StartSplitter(VERTICAL);
    SableUI::StartSplitter(HORIZONTAL);
    SableUI::AddPanel();
    SableUI::StartSplitter(VERTICAL);
    SableUI::AddPanel();
    SableUI::StartSplitter(HORIZONTAL);
    SableUI::AddPanel();
    SableUI::AddPanel();
    SableUI::EndSplitter();
    SableUI::EndSplitter();
    SableUI::EndSplitter();
    SableUI::AddPanel();
    SableUI::EndSplitter();
    SableUI::StartSplitter(VERTICAL);
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