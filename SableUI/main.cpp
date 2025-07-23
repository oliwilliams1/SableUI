#include "SableUI/SableUI.h"

#define VSPLITTER SableUI::NodeType::VSPLITTER
#define HSPLITTER SableUI::NodeType::HSPLITTER
#define COMPONENT SableUI::NodeType::BASE
#define Colour SableUI::Colour
#define Color SableUI::Colour

int main(int argc, char** argv)
{
    SableUI::Window mainWindow("SableUI Layout Test", 1600, 1000);

    // Create the root node
    SableUI::RootNode* rootNode = mainWindow.GetRoot();

    // Add the root horizontal splitter
    SableUI::SplitterNode* rootHSplitter = rootNode->AddSplitter(HSPLITTER);
    if (rootHSplitter)
    {
        // Left side of RootHSplitter: Your original main VSplitter structure
        SableUI::SplitterNode* vsplitter1 = rootHSplitter->AddSplitter(VSPLITTER);
        if (vsplitter1)
        {
            // Adding TopHSplitter with splitter attachment
            SableUI::SplitterNode* hsplitter1 = vsplitter1->AddSplitter(HSPLITTER);
            if (hsplitter1)
            {
                hsplitter1->AddBaseNode(); // TopLeftComponent

                // Adding NestedVSplitter with splitter attachment
                SableUI::SplitterNode* vsplitter2 = hsplitter1->AddSplitter(VSPLITTER);
                if (vsplitter2)
                {
                    vsplitter2->AddBaseNode(); // NestedTopComponent

                    // Adding NestedHSplitter with splitter attachment
                    SableUI::SplitterNode* hsplitter2 = vsplitter2->AddSplitter(HSPLITTER);
                    if (hsplitter2)
                    {
                        hsplitter2->AddBaseNode(); // NestedBottomLeftComponent
                        hsplitter2->AddBaseNode(); // NestedBottomRightComponent
                    }
                }
            }

            // Adding MainBottomComponent with splitter attachment
            vsplitter1->AddBaseNode(); // MainBottomComponent
        }

        // Right side of RootHSplitter: A brand new VSplitter branch
        SableUI::SplitterNode* rightVSplitter = rootHSplitter->AddSplitter(VSPLITTER);
        if (rightVSplitter)
        {
            // Component at the top of the right VSplitter
            rightVSplitter->AddBaseNode(); // RightTopComponent

            // Component at the bottom of the right VSplitter
            rightVSplitter->AddBaseNode(); // RightBottomComponent
        }
    }

    // --- Main Loop ---
    while (mainWindow.PollEvents())
    {
        mainWindow.Draw();
    }

    delete rootNode; // Clean up the root node
    return 0;
}