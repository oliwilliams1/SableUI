#include "SableUI/SableUI.h"

#define VSPLITTER SableUI::NodeType::VSPLITTER
#define HSPLITTER SableUI::NodeType::HSPLITTER
#define COMPONENT SableUI::NodeType::BASE
#define Colour SableUI::Colour
#define Color SableUI::Colour

int main(int argc, char** argv)
{
    SableUI::Window mainWindow("SableUI Layout Test", 1600, 1000);

    // Root = nullptr
    SableUI::SplitterNode* rootHSplitter = mainWindow.AddSplitter(nullptr, HSPLITTER, "RootHSplitter");
    if (rootHSplitter)
    {
        // Left side of RootHSplitter: Your original main VSplitter structure
        SableUI::SplitterNode* vsplitter1 = mainWindow.AddSplitter(rootHSplitter, VSPLITTER, "MainVSplitter");
        if (vsplitter1)
        {
            // Adding TopHSplitter with splitter attachment
            SableUI::SplitterNode* hsplitter1 = mainWindow.AddSplitter(vsplitter1, HSPLITTER, "TopHSplitter");
            if (hsplitter1)
            {
                mainWindow.AddBaseNode(hsplitter1, "TopLeftComponent");

                // Adding NestedVSplitter with splitter attachment
                SableUI::SplitterNode* vsplitter2 = mainWindow.AddSplitter(hsplitter1, VSPLITTER, "NestedVSplitter");
                if (vsplitter2)
                {
                    mainWindow.AddBaseNode(vsplitter2, "NestedTopComponent");

                    // Adding NestedHSplitter with splitter attachment
                    SableUI::SplitterNode* hsplitter2 = mainWindow.AddSplitter(vsplitter2, HSPLITTER, "NestedHSplitter");
                    if (hsplitter2)
                    {
                        mainWindow.AddBaseNode(hsplitter2, "NestedBottomLeftComponent");
                        mainWindow.AddBaseNode(hsplitter2, "NestedBottomRightComponent");
                    }
                }
            }

            // Adding MainBottomComponent with splitter attachment
            mainWindow.AddBaseNode(vsplitter1, "MainBottomComponent");
        }

        // Right side of RootHSplitter: A brand new VSplitter branch
        SableUI::SplitterNode* rightVSplitter = mainWindow.AddSplitter(rootHSplitter, VSPLITTER, "RightVSplitter");
        if (rightVSplitter)
        {
            // Component at the top of the right VSplitter
            mainWindow.AddBaseNode(rightVSplitter, "RightTopComponent");

            // Component at the bottom of the right VSplitter
            mainWindow.AddBaseNode(rightVSplitter, "RightBottomComponent");
        }
    }

	// --- Main Loop ---
	while (mainWindow.PollEvents())
	{
        
		mainWindow.Draw();
	}

	return 0;
}