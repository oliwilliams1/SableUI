# SableUI
A high-performance and low-cost UI library inspired by foobar2000's ColumnsUI 
component-based UI extenstion, written in C++. Windows will be defined in with 
multiple "nodes", which can be thought of as widgets, each containing unique
and seperated content from other nodes. Each node can be set as splitters or 
components, splitters allow multiple children nodes in a certain direction
and components allow elements to be attached. Think of each "component" node 
as an html canvas, but much more efficient as no webview enviroments are needed.

## Features
- **Component-Based layout**: Easy management of a modular UI system composed of nodes
- **Flexiblity**: Like foobar2000's columns ui, splitters can be used to create complex dynamic layouts of components
- **Efficiency and low-cost performance**: With an idle of no resource usage & 30mb ram if you have a dedicated GPU, 
  in comparision to inegrating a web-based solution like webview, this approach is much more performant
- **Complex text rendering**: Harnessing freetype and efficient font rendering techniques, unicode support
  is built in and efficient caching is always in use
- **Hardware accelerated**: Using the GPU for graphical tasks instead of software rendering
  offers a huge boost in performance and efficiency, and by harnessing newer and modern graphics
  API's like Vulkan, memory management has never been so strict and efficient.

## Building from Source
Ensure you have CMake installed on your machine with a C++ compiler like MSVC 
for windows, gcc for Linux, MacOS. C++ Compiler must be atleast C++17 due to use
of the filesystem standard library among other modern features.

To ensure all third-party libraries are included, clone this repository with 
the recursive option, otherwise compilation will not work!

```bash
git clone https://github.com/oliwilliams1/SableUI --recursive
cd SableUI
```

### Windows
1. Run ```setup.bat``` to generate project files for Microsoft Visual Studio via CMake.
2. Navigate to the build directory and open ```SableUI.sln```.
3. In Solution Explorer, set ```SableUI``` as the startup project if you are running the demo, skip step if building libraries.
4. Build and Run!

### Linux / macOS
1. Create a build directory and navigate into it:
```bash
mkdir build
cd build
```

2. Run CMake to configure the project:
```bash
cmake ..
```
3. Compile the project using ```make```. You can use the ```-j``` flag for faster compilation (e.g., ```-j12```):
```bash
make -j12
```
4. Run the application (if running demo)
```bash
./SableUI
```

## Basic use example
```cpp
#include "SableUI.h"

int main()
{
	// Create window, with instance attached to a variable
	SableUI::Window mainWindow("SableUI Example", 800, 600);

	// Open xml-based file for ui to configure nodes, programatically doc to be added later
	// See layout of an sbml file in readme
	SableUI::OpenUIFile("example.sbml");

	// To add elements (via a sbml is not implemented yet) you have to do programatically
	// Api to add elements is struct-based, inspired by vulkan api
	SableUI::ElementInfo el1{};                  // Initialise empty struct for filling in
	el1.name      = "element 1";                 // Custom name to acceess later, simmilar to js getElementByID method
	el1.wType     = SableUI::RectType::FILL;     // Fit and share space with other elements
	el1.hType     = SableUI::RectType::FIXED;    // Allow definition of height, cannot be resized automatically
	el1.height    = 20.0f;                       // Set the fixed height of this element
	el1.bgColour  = SableUI::Colour(255, 0, 0);  // Red background
	
	// Call a method of the window to add an element to it, and element is added to a componenent, defined by name
	// Of type ElementType::RECT (meaning can have children and by default, renders the background)
	mainWindow.AddElementToComponent("component 3", el1, SableUI::ElementType::RECT);

	// Adding an image
	SableUI::ElementInfo imel1{};
	imel1.name    = "image element";
	imel1.wType   = SableUI::RectType::FIXED;    // Fixed aspect ratio, image will not be stretched
	imel1.hType   = SableUI::RectType::FIXED;    // Fixed aspect ratio, image will not be stretched
	imel1.padding = 5.0f;                        // Padding
	imel1.centerX = true;                        // Center it horizontally in relation to parent
	imel1.width   = 128.0f;                      // Set displayed image width and height
	imel1.height  = 128.0f;                      // Set displayed image width and height
	// Attach element to "component 3", of type ElementType::IMAGE to enable image capabilities
	SableUI::Element* imageElement = mainWindow.AddElementToComponent("component 3", imel1, SableUI::ElementType::IMAGE);
	imageElement->SetImage("image.png");
	/* Must call SetImage AFTER init, if program cannot find "image.png", it will display a null 
	   texture and display a warning message in console */

	// Adding a text element
	SableUI::ElementInfo el5{};
	el5.name      = "text element";
	el5.wType     = SableUI::RectType::FILL;     // Max width of parent element
	el5.hType     = SableUI::RectType::FIXED;    // Max height is fixed
	el5.height    = 24.0f;                       // Max height of 24px
	// Attach element to "component 3", of type ElementType::TEXT to enable text capabilities
	SableUI::Element* textElement = mainWindow.AddElementToComponent("component 3", el5, SableUI::ElementType::TEXT);
	textElement->SetText(U"Hello 123 | (#♥) | (＜★) | 이브, 프시케 🎉", 24);
	/* Must call SetText AFTER init, note SetText takes in a unicode string not a normal string, this allows
	   for complex strings, default font pack should contain most characters. When rendering this text, if it 
	   cannot find a special character in default atlas, it will search the fonts/ directory for it, and cache it.
	   if special character still cannot be found, a warning message will display in console */

	// Basic usage lifetime, loop waits for an event before drawing
	while (SableUI::PollEvents())
	{
		// Draw all components when something changed
		SableUI::Draw();
	}

	// Exit safely
	SableUI::Destroy();
}
```

### ElementInfo Struct
The ```ElementInfo``` struct allows for flexible configuration of UI elements:
```cpp
struct ElementInfo
{
    std::string name;                        // Custom name, used for attaching, editing components without a saved reference
    Colour bgColour = Colour(128, 128, 128); // Background colour for solid rect elements
    float xOffset = 0;                       // Horizontal offset from left side
    float yOffset = 0;                       // Vertical offset from top
    float width = 0;                         // Width in px, only useful if wType == RectType::FIXED
    float height = 0;                        // Height in px, only useful if hType == RectType::FIXED
    float padding = 0;                       // Inner padding of elements, useful when adding children elements
    bool centerX = false;                    // Is centered horizontally?
    bool centerY = false;                    // Is centered vertically?
    RectType wType = RectType::FILL;         // Can be FILL or FIXED, use fill to enable automatic dynamic resizing, fixed for fixed-width elements
    RectType hType = RectType::FILL;
    ElementType type = ElementType::UNDEF;   // Can be UNDEF (for init error checks), RECT (for solid rectangle elements), IMAGE, or TEXT
};
```

### Example .sbml File
Example of an xml-based .sbml file to configure component layout, elements are yet to be supported
```xml
<!-- example.sbml -->
<root> <!-- Root must be declared for main layout, can only have one child -->
  <hsplitter> <!-- Solid component on left (first declared), vertical splitter with 3 components on right -->
    <component colour="(255, 0, 255)" />
    
    <vsplitter> <!-- 3 Vertical components stacked on one another -->
      <component colour="(0, 255, 0)" />

      <hsplitter> <!-- Middle component with 2 components split on vertical axis (one on right, one on left) -->
        <component colour="(0, 128, 128)" />
        <component colour="(128, 0, 128)" />
      </hsplitter>

      <component colour="(255, 255, 0)" />
    </vsplitter>
  </hsplitter>
</root>
```

## Acknowledgments

This project uses the following libraries:
- [glfw](https://github.com/glfw/glfw) For simple cross-platform window management for backends of OpenGL and Vulkan
- [FreeType](https://www.freetype.org) A library that is litterally the backbone Android, Linux, Chrome, and many more 
  for its complex text rendering capabilities, which SableUI adopts
- [stb_image + stb_image_resize2](https://github.com/nothings/stb) Image handling (loading & resizing) without hassle
- [TinyXML2](https://github.com/leethomason/tinyxml2) For handling the custom .smbl xml-based language
- [glew](https://github.com/nigels-com/glew) GL extention wrangler for modern OpenGL features for the OpenGL backend
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) Interfacing with a much lower-level modern GPU API, this program uses
  for more efficient and lower cost usage of this program