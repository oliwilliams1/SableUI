#include "SableUI/SableUI.h"

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent(int r) : SableUI::BaseComponent(Colour(r, 32, 32)), v(r) {};

    void Init() override
    {
        SableUI::LayoutDirection dir = (v == 128) ? SableUI::LayoutDirection::RIGHT_LEFT : SableUI::LayoutDirection::LEFT_RIGHT;

        RectContainer(.setLayoutDirection(dir) bg(255, 0, 0))
            RectContainer(id("child") w(50) h(50) bg(0, 255, 0))
                Rect(px(5) py(5) bg(0, 255, 255) w(50) h(50));
            Rect(centerXY w(20) mt(4) h(20) bg(0, 0, 255));
            EndContainer

            RectContainer(m(5) w(100) h(100) bg(255, 255, 0))
                Rect(px(5) py(5) bg(255, 0, 255) w(50) h(50));
            EndContainer
        EndContainer

        Rect(m(15) w_fill h(75) bg(128, 128, 128));
        Rect(m(5) w(60) h(60) bg(255, 128, 0));
    }
private:
    int v;
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
                    topLeftComponent->AttachComponent<TestComponent>(128);
                }

                if (SableUI::SplitterPanel* vsplitter2 = SableUI::StartSplitter(SableUI::PanelType::VERTICAL))
                {
                    SableUI::Panel* nestedTopComponent = SableUI::AddPanel();
                    if (SableUI::SplitterPanel* hsplitter2 = SableUI::StartSplitter(SableUI::PanelType::HORIZONTAL))
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
        if (SableUI::SplitterPanel* rightVSplitter = SableUI::StartSplitter(SableUI::PanelType::VERTICAL))
        {
            SableUI::Panel* rightTopComponent = SableUI::AddPanel();
            SableUI::Panel* rightBottomComponent = SableUI::AddPanel();

            SableUI::EndSplitter(); // RightVSplitter
        }

        SableUI::EndSplitter(); // RootHSplitter
    }

    while (mainWindow.PollEvents())
    {
        mainWindow.Draw();
    }

    return 0;
}