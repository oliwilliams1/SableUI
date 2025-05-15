# SableUI
A high-performance and low-cost UI library inspired by foobar2000's ColumnsUI component-based UI extenstion, written in C++.

## Building from Source
Ensure you have CMake installed on your machine with a C++ compiler like MSVC for windows, gcc for Linux, MacOS.

To ensure all third-party libraries are included, clone this repository with the recursive option:

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
	// Can add two params at end of function defining window pos
	SableUI::CreateWindow("SableUI", 800, 600);

	// Open xml-based file for ui to configure itself
	SableUI::OpenUIFile("template.SableUI");

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

```xml
<!-- template.SableUI -->
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
### TinyXML2
- **Attribution**: TinyXML2 is a simple, small, efficient, C++ XML parser that can be easily integrated into various projects.
- **Website**: [TinyXML2 GitHub](https://github.com/leethomason/tinyxml2)

### SDL2
- **Attribution**: SDL2 (Simple DirectMedia Layer) is a cross-platform multimedia library designed to provide low-level access to audio, keyboard, mouse, joystick, and graphics hardware.
- **Website**: [SDL2 Website](https://www.libsdl.org)

### FreeType
- **Attribution**: FreeType is a freely available software library to render fonts.
- **Website**: [FreeType Website](https://www.freetype.org)

### stb, stb_image
- **Attribution**: stb_image is a single-file public domain library for loading images in various formats.
- **Website**: [stb_image GitHub](https://github.com/nothings/stb)