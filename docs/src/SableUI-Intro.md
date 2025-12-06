# What is SableUI?

SableUI is a high-performance C++ UI/application framework that reimagines native UI development by borrowing the best ideas from modern web frameworks.

### React-Inspired Components
Components in SableUI work just like React components:
```cpp
class MyComponent : public SableUI::BaseComponent {
public:
    void Layout() override {
        // Element tree starts here
        Div(bg(32, 32, 32) p(10)) {
            Text("Hello, World!");
        }
    }
};
```

Every component has a `Layout()` method that can be overriden that describes what should be rendered. When state changes through `useState()` or otherwise, SableUI manages to only what's necessary through virtual DOM reconciliation.

### Declarative, Composable UI

Instead of the traditional approach which can get messy with complex layouts:
```cpp
ElementInfo divInfo{};
divInfo.backgroundColuor = Colour(32, 32, 32);
divInfo.padding = 10;
StartDiv(info);
AddText("Hello world");
EndDiv();
```

SableUI utilises macros and RAII to abstract the complexities that come with low-level ui libraries remaing safe and impossible to forget matching a `End()` to a `Start()` in the process:
```cpp
Div(bg(32, 32, 32) p(10)) {
    Text("Hello, World!");
}
```

Both methods however are supported in SableUI, but for more complex layouts, macros are provided to make layouts more readable, easy, and safe. You can understand how to layout elements here. <- TO ADD and the legacy method here. <- TO ADD

### Tailwind-Inspired Styling
Styling is fast and intuitive with chainable modifiers: <- adding link to styling page
```cpp
Div(
    w(200) h(100)       // Fixed Width and height
    bg(45, 45, 45)      // Background color
    p(10)               // Padding (all around)
    m(5)                // Margin (all around)
    rounded(8)          // Border radius (all corners)
    centerXY            // Center content in both axis
)
```

### Flexible Panel-Based Layouts
SableUI includes a powerful splitter panel system for complex layouts:
```cpp
int main() {
    SableUI::Window* window = SableUI::Initialise("My App", 1200, 800);
    
    HSplitter() {  // Horizontal splitter
        VSplitter() {  // Vertical splitter on the left
            Panel("FileTree");
            Panel("Editor");
        }
        Panel("Console");  // Console on the right
    }
    
    while (SableUI::PollEvents())
    {
        SableUI::Render();
    }
    
    SableUI::Shutdown();
    return 0;
}
```

Panels can be resized at runtime (and in the future save their state across multiple sessions), and you can set minimum/maximum bounds for scale-sensitive panels.

## Couple of noteable features
### Component State
Use `useState` to create reactive state:
```cpp
class Counter : public SableUI::BaseComponent {
    void Layout() override {
        Text(SableString::Format("Count: %d", count));
        
        Div(onClick([this]() { setCount(count + 1); })) {
            Text("Increment");
        }
    }
private:
    useState(count, setCount, int, 0);
    /* Same as following for react, but implemented *
     * as a macro due to limitations within c++     *
     * const [count, setCount] = useState<int>(0);  */
};
```

### Virtual DOM
SableUI utilises a virtual DOM for comparing new/old trees, and only updating and rendering the changes between them. Reconciliation only occurs on components whose state has been changed or has been manually flagged for reconciliation. This ensures good performance for reactive states, and maintain flexibility for users.

### Text Rendering
Text rendering is advanced, as if something is even subtly wrong or unsupported it can make an applciation awkward to use, and users would unconsiously prefer web-based apps to ensure good text rendering. Below are features SableUI supports:
- Full Unicode support (including emoji (greyscale only), CJK characters, any range supplied by the bundled font packs)
- Multiple font styles (bold, italic, light)
- Word wrapping and truncation
- LCD subpixel rendering for legibility on older displays
- Cached glyph atlases for performance

## Graphics Backend Support
Currently supported:
- **OpenGL 3.3+** (Fully implemented)

Planned:
- **Vulkan** (In progress)
- **Metal** (Will come with VK via MoltenVK)

You can select the preferred backend at initialisation:
```cpp
SableUI::SetBackend(SableUI::Backend::OpenGL);
// or via command line: --opengl, --vulkan
```
> The backend selection ships with the program. I.e. a device compiling the library without the VulkanSDK will not be able to use ship vulkan compatability with their application, for that there will be precompiled binaries (not as of current though).

## Platform Support
- **Windows** (MSVC, WSL) MinGW untested
- **Linux** (WSL) GCC & Clang untested
- **macOS** Untested

## What's Next?
If SableUI sounds right for you, you can now [get started](Quick-Start.md) with building your first application!

Or explore:
- **Core Concepts**
- **Examples**
- **API Reference**