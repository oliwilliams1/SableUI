#include "SableUI/SableUI.h"

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent(int r) : SableUI::BaseComponent(Colour(r, 32, 32)), v(r) {};

    void Init() override
    {
        SableUI::LayoutDirection dir = (v == 128) ? SableUI::LayoutDirection::RIGHT_LEFT : SableUI::LayoutDirection::LEFT_RIGHT;

        Div(.setLayoutDirection(dir) bg(255, 0, 0))
        {
            Div(id("child") w(50) h(50) bg(0, 255, 0))
            {
                Rect(px(5) py(5) bg(0, 255, 255) w(50) h(50));
                Rect(centerY w(20) mt(4) h(20) bg(0, 0, 255));
            }

            Div(m(5) w(100) h(100) bg(255, 255, 0))
            {
                Rect(px(5) py(5) bg(255, 0, 255) w(50) h(50));
            }
        }

        Rect(m(15) w_fill h(75) bg(128, 128, 128));
        Rect(m(5) w(60) h(60) bg(255, 128, 0));

    }
private:
    int v;
};

class ImageView : public SableUI::BaseComponent
{
public:
    ImageView(const std::string& path, int width = 128, int height = 128) : SableUI::BaseComponent(), width(width), height(height), m_path(path) {};

    void Init() override
    {
        UpdateStyle(rootElement, bg(30, 30, 30));

        Image(m_path, w(width) h(height) centerXY);
        Text(m_path);
    }

private:
    std::string m_path;
    int width, height;
};

int main(int argc, char** argv)
{
    using namespace SableUI;

    Window mainWindow("SableUI Layout Test", 1600, 1000);
    SetContext(&mainWindow);

    if (SplitterPanel* rootHSplitter = StartSplitter(PanelType::HORIZONTAL))
    {
        if (SplitterPanel* vsplitter1 = StartSplitter(PanelType::VERTICAL))
        {
            if (SplitterPanel* hsplitter1 = StartSplitter(PanelType::HORIZONTAL))
            {
                Panel* topLeftComponent = AddPanel()->AttachComponent<TestComponent>(128);

                if (SplitterPanel* vsplitter2 = StartSplitter(PanelType::VERTICAL))
                {
                    Panel* nestedTopComponent = AddPanel();
                    if (SplitterPanel* hsplitter2 = StartSplitter(PanelType::HORIZONTAL))
                    {
                        Panel* nestedBottomLeftComponent = AddPanel()->AttachComponent<TestComponent>(80);
                        Panel* nestedBottomRightComponent = AddPanel()->AttachComponent<ImageView>("dirtywork.jpg", 128, 128);

                        EndSplitter(); // HSplitter
                    }

                    EndSplitter(); // VSplitter
                }

                EndSplitter(); // TopHSplitter
            }
            Panel* mainBottomComponent = AddPanel();

            EndSplitter(); // MainVSplitter
        }
        if (SplitterPanel* rightVSplitter = StartSplitter(PanelType::VERTICAL))
        {
            Panel* rightTopComponent = AddPanel();
            Panel* rightBottomComponent = AddPanel();

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