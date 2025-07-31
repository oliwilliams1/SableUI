#include "SableUI/SableUI.h"

#define VSPLITTER SableUI::NodeType::VSPLITTER
#define HSPLITTER SableUI::NodeType::HSPLITTER
#define COMPONENT SableUI::NodeType::BASE
#define Colour SableUI::Colour
#define Color SableUI::Colour

class TestComponent : public SableUI::BaseComponent
{
public:
    TestComponent() : SableUI::BaseComponent(Colour(80, 32, 32)) {};
};

int main(int argc, char** argv)
{
    SableUI::Window mainWindow("SableUI Layout Test", 1600, 1000);

    SableUI::RootNode* rootNode = mainWindow.GetRoot();

    SableUI::SplitterNode* rootHSplitter = rootNode->AddSplitter(HSPLITTER);
    if (rootHSplitter)
    {
        SableUI::SplitterNode* vsplitter1 = rootHSplitter->AddSplitter(VSPLITTER);
        if (vsplitter1)
        {
            SableUI::SplitterNode* hsplitter1 = vsplitter1->AddSplitter(HSPLITTER);
            if (hsplitter1)
            {
                hsplitter1->AddBaseNode()->AttachComponent<TestComponent>();

                SableUI::SplitterNode* vsplitter2 = hsplitter1->AddSplitter(VSPLITTER);
                if (vsplitter2)
                {
                    vsplitter2->AddBaseNode();

                    SableUI::SplitterNode* hsplitter2 = vsplitter2->AddSplitter(HSPLITTER);
                    if (hsplitter2)
                    {
                        hsplitter2->AddBaseNode();
                        hsplitter2->AddBaseNode();
                    }
                }
            }

            vsplitter1->AddBaseNode();
        }

        SableUI::SplitterNode* rightVSplitter = rootHSplitter->AddSplitter(VSPLITTER);
        if (rightVSplitter)
        {
            rightVSplitter->AddBaseNode();

            rightVSplitter->AddBaseNode();
        }
    }

    /* main loop */
    while (mainWindow.PollEvents())
    {
        mainWindow.Draw();
    }

    delete rootNode;
    return 0;
}