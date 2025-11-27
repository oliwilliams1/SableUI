# SableUI
A high-performance C++ UI/application framework combining React's component
and state model, Tailwind's easy styling approach, and foobar2000's ColumnsUI
flexible panel-based layouts.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-green)
![Status](https://img.shields.io/badge/Status-Work%20in%20Progress:%20~65%25-yellow)

## Features
- Declarative components with reactive state management
- Flexible layouts with resizable splitter panels
- Advanced text rendering with full unicode support, custom styling, and colour
- Cross-platform compatibility supporting OpenGL (soon supporting Vulkan, DirectX, Metal, GLES)
  - Supports Windows, Linux, and macOS (Intel & Apple Silicon)
  <br>macOS untested, but should compile due to Linux support

<br>
<details>
    <summary>More examples</summary>
    <img src="readme-resources/complex-demo.png">
</details>

### Example Usage: Counter App
![Example counter app](readme-resources/counter-app-example.png)
```cpp
#include <SableUI/SableUI.h> // include core library
#include <SableUI/components/debugComponents.h> // for element inspector

class Counter : public SableUI::BaseComponent
{
public:
    Counter() : SableUI::BaseComponent() {}

    void Layout() override
    {
        // outer container
        Div(bg(245, 245, 245) p(30) centerXY w_fit h_fit rounded(10)) // simmilar to tailwind styling
        {
            // formatting strings made easy for text elements
            Text(SableString::Format("Count: %d", count),
                fontSize(28) mb(20) textColour(20, 20, 20) justify_center);

            // horizontal row with buttons
            Div(left_right p(4) centerX rounded(9))
            {
                Div(bg(90, 160, 255) p(8) mr(5) rounded(5)
                    onClick([=]() { setCount(count + 1); })) // lambda event callbacks
                {
                    Text("Increment",
                        mb(2) textColour(255, 255, 255) fontSize(16) justify_center);
                }

                Div(bg(255, 120, 120) p(8) rounded(5)
                    onClick([=]() { setCount(count - 1); }))
                {
                    Text("Decrement",
                        mb(2) textColour(255, 255, 255) fontSize(16) justify_center);
                }
            }
        } // macro utilises RAII for proper tree generation and lifetime handling
    }

private:
    /* in react this would be written as:
     * const [count, setCount] = useState<int>(0);
     * but due to limitations in c++, have to use a macro */
    useState(count, setCount, int, 0);
};


int main(int argc, char** argv)
{
    SableUI::PreInit(argc, argv); // not neccasary, but there for to switch backends through args
    SableUI::Window* window = SableUI::Initialise("SableUI App", 800, 600);

    VSplitter()
    {
        PanelWith(Counter);
    }

    // create an element inspector in a different window
    SableUI::CreateSecondaryWindow("Debug View", 250, 600);
    VSplitter()
    {
        PanelWith(SableUI::ElementTreeView, window); // focus around this window
        PanelWith(SableUI::PropertiesView);
    }

    while (SableUI::PollEvents())
        SableUI::Render();

    SableUI::Shutdown();
    return 0;
}
```
---
## Building
Requires CMake for creating build files and a C++17 compiler.

> NOTE: If building from source you may need development libraries for your
specific graphics api otherwise CMake will not find libraries and may not be
able to build with graphics API support, to avoid this check for prebuilt
binaries in releases (not uploaded until project completion).

Clone with submodules
```bash
git clone https://github.com/oliwilliams1/SableUI --recursive
cd SableUI
```
#### Windows
```bash
mkdir build && cd build
setup.bat
```
**Compile instructions for MSVC:**<br>
Open the ```.sln``` in Visual Studio, set ```SableUI``` as current target and build.
#### Linux/macOS
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Acknowledgments
This project uses the following open-source libraries:
- [GLFW](https://github.com/glfw/glfw) - Window & input handling
- [GLEW](https://github.com/nigels-com/glew) - OpenGL extensions
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) - Vulkan graphics API
- [FreeType](https://www.freetype.org) - Font rendering
- [libwebp](https://github.com/webmproject/libwebp) - WebP image support
- [Shaderc](https://github.com/google/shaderc) - Cross-platform shader compilation
- [stb_image + stb_image_resize2](https://github.com/nothings/stb) - Image loading

Licenses for these libraries can be found in their submodule directories as their source is linked to this library (vendor/)