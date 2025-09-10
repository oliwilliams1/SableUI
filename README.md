# INCOMPLETE
# SableUI
A high-performance and low-cost UI library inspired by foobar2000's ColumnsUI 
component-based UI extenstion, written in C++. Windows will be defined in with 
multiple "nodes", which can be thought of as widgets, each containing unique
and seperated content from other nodes. Each node can be set as splitters or 
components, splitters allow multiple children nodes in a certain direction
and components allow elements to be attached. Think of each "component" node 
as an html canvas, but much more efficient as no webview enviroments are needed.

SableUI is a modern, low-level and high-performance, application UI framework
for C++ drawing inspiration from Reacts' dynamic component-based "immediate mode"
gui, tailwinds simple styling, and foobar2000s ColumnsUI panel-based window structure.

## Features
- Easy API Usage: Utilising macros etc
- Hardware Accel: Lightweight, low-level
- Cross-plaftform

### Example Usage
```cpp
#include "SableUI/SableUI.h"

class HoverImageView : public SableUI::BaseComponent
{
public:
	HoverImageView() : SableUI::BaseComponent() {};

	void Layout() override
	{
		std::string path = (isHovered) ? "img1.webp" : "img2.jpg";

		Div(w(128) h(160))
		{
			Div(bg(128, 32, 32)
				onHover([&]() { setIsHovered(true); })
				onHoverExit([&]() { setIsHovered(false); }))
			{
				Text("Hover to change image,\nloaded: " + path);
			}

			Image(path, w(128) h(128));
		}
	}

private:
	useState(isHovered, setIsHovered, bool, false);
};

int main(int argc, char** argv)
{
	using namespace SableUI;
	Window mainWindow("SableUI Layout Test", 1600, 1000);
	SetContext(&mainWindow);
	
	HSplitter()
	{
		PanelWith(HoverImageView);
	}

	while (mainWindow.PollEvents())
	{
		mainWindow.Draw();
	}
	return 0;
}
```

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

## Acknowledgments
This project uses the following libraries:
- [glfw](https://github.com/glfw/glfw)
- [glew](https://github.com/nigels-com/glew)
- [Vulkan](https://www.lunarg.com/vulkan-sdk/)
- [FreeType](https://www.freetype.org)
- [libwebp](https://github.com/webmproject/libwebp)
- [stb_image + stb_image_resize2](https://github.com/nothings/stb)