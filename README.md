# SableUI
A high-performance and low-cost UI library inspired by foobar2000's ColumnsUI component-based UI extenstion, written in C++.

## Libraries used
[SDL2](https://github.com/libsdl-org/SDL/tree/SDL2)

Planned: [stb_image](https://github.com/nothings/stb)

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
3. In Solution Explorer, set SableUI as the startup project if you are running the demo, skip step if building libraries.
4. Build!

### Linux / macOS
1. Create a build directory and navigate into it:
```bash
mkdir build
cd build
```

2. Run CMake to configure the project:
```
cmake ..
```
3. Compile the project using ```make```. You can use the ```-j``` flag for faster compilation (e.g., ```-j12```):
```
make -j12
```
4. Run the application (if running demo)
```
./SableUI
```

## Basic use example
```
#include "SableUI.h"

int main()
{
	// Can add two params at end of function defining window pos
	SableUI::CreateWindow("SableUI", 800, 600);

	/* Maximum FPS so draws do not submitted at a rate too fase for monitors,
	   a way of keeping this UI framework lightweight */
	SableUI::SetMaxFPS(60);

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