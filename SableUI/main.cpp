#include "SableUI/SableUI.h"

#define VERTICAL SableUI::PanelType::VERTICAL
#define HORIZONTAL SableUI::PanelType::HORIZONTAL
#define Colour SableUI::Colour
#define Color Colour

class RectGuard
{
public:
    explicit RectGuard(const SableUI::ElementInfo& info)
    {
        SableUI::StartRect(info);
    }
    ~RectGuard()
    {
        SableUI_Log("Destructor called");
        SableUI::EndRect();
    }
    RectGuard(const RectGuard&) = delete;
    RectGuard& operator=(const RectGuard&) = delete;
    RectGuard(RectGuard&&) = default;
    RectGuard& operator=(RectGuard&&) = default;
};

#define Rect(...) auto CONCAT(_rect_guard_, __LINE__) = RectGuard(SableUI::ElementInfo{} __VA_ARGS__)

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define w(value)  .setWidth(value).setWType(SableUI::RectType::FIXED)
#define h(value)  .setHeight(value).setHType(SableUI::RectType::FIXED)
#define bg(...)   .setBgColour(Colour(__VA_ARGS__))

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent(int r) : SableUI::BaseComponent(Colour(r, 32, 32)) {};

    void Init() override
    {
        Rect(w(200) h(200) bg(255, 0, 0));
        {
            Rect(w(50) h(50) bg(0, 255, 0));
            {
                Rect(w(20) h(20) bg(0, 0, 255));
            }
        }
        Rect(w(75) h(75) bg(255, 255, 0));
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