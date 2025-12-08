# SableUI
A high-performance C++ UI/application framework combining React's component
and state model, Tailwind's easy styling approach, and foobar2000's ColumnsUI
flexible panel-based layouts.

![C++17](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-green)
![Status](https://img.shields.io/badge/Status-Work%20in%20Progress:%20~70%25-yellow)

## Features
- **Declarative components** with reactive state management
- **Flexible layouts** with resizable splitter panels
- **Advanced text rendering** with full unicode support, custom styling, and colour
- **Cross-platform** compatibility supporting OpenGL (soon supporting Vulkan, DirectX, Metal, GLES)
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
#include <SableUI/SableUI.h>

class Counter : public SableUI::BaseComponent {
public:
    void Layout() override {
        Div(bg(245, 245, 245) p(30) centerXY w_fit h_fit rounded(10)) {
            Text(SableString::Format("Count: %d", count),
                fontSize(28) mb(20) textColour(20, 20, 20) justify_center);

            Div(left_right p(4) centerX rounded(9)) {
                Div(bg(90, 160, 255) p(8) mr(5) rounded(5)
                    onClick([this]() { setCount(count + 1); })) {
                    Text("Increment", 
                        textColour(255, 255, 255) fontSize(16) justify_center);
                }

                Div(bg(255, 120, 120) p(8) rounded(5)
                    onClick([this]() { setCount(count - 1); })) {
                    Text("Decrement",
                        textColour(255, 255, 255) fontSize(16) justify_center);
                }
            }
        }
    }

private:
    /* in react this would be written as:
     * const [count, setCount] = useState<int>(0);
     * but due to limitations in c++, have to use a macro */
    useState(count, setCount, int, 0);
};


int main(int argc, char** argv) {
    // Register your component
    SableUI::RegisterComponent<Counter>("Counter");

    // Initialize window
    SableUI::Window* window = SableUI::Initialise("Counter App", 800, 600);

    // Create layout
    Panel("Counter");

    // Main loop
    while (SableUI::PollEvents())
        SableUI::Render();

    SableUI::Shutdown();
    return 0;
}

```
---

## Building and Using library
This project has a [documentation](sableui.oliwilliams.dev/getting-started) with a getting started gyude to build the basic app seen above. Precompiled binaries are not released until v1.0 as things are subject to change.

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