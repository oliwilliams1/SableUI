# What is SableUI?
The driving force behind SableUI is to solve a problem with modern UI development. When building an application, web technologies are often preferable, but they come with heavy performance costs and annoying abstraction layers. SableUI brings modern UI development to a lower level with zero runtime overhead.

## Core Philosophy
Traditional C++ UI frameworks require verbose code that's often hard to read. Web frameworks like react solved this with declarative components, but at the cost of performance and introduces an abstraction layer between UI and application logic. SableUI attempts to bridge this gap.

**Traditional C++ Pseudo-code**
```cpp
auto* button = new Button();
button->setText("Click me");
button->setPosition(10, 10);
button->onClick([&]() { count++; updateLabel(); });
layout->addWidget(button);
// OR
ElementInfo info{};
info.label = "Click me";
info.position = vec2(10, 10);
info.onClick = [&]() { count++; updateLabel(); };
AddButton(info);
```
These solutions are not reactive, meaning manual re-renders will have to be scripted to update the labels, and with more complex heirachies it becomes impossible to manage.

**SableUI:**
```cpp
Div(onClick([this]() { setCount(count + 1); })) {
    Text("Click me");
}
```
Concise and reactive (comparable to react), `setCount()` marks the element as dirty, and will be automatically rerendered next frame.

## Key Features
### React-Inspired components
Components describe what should be rendered, not requireing definitions on how to update existing UI. State changes trigger automatic efficient rerenders through reconcilliation with a virtual DOM.
```cpp
class TodoList : public SableUI::BaseComponent {
    void Layout() override {
        for (const auto& todo : todos) {
            Div(bg(45, 45, 45) p(10) mb(5)) {
                Text(todo.text);
            }
        }
    }
private:
    // When "todos" changes via "setTodos()", this component will be automatically re-rendered
    useState(todos, setTodos, std::vector<Todo>, {});
    /* ^^ This syntax is equivilant to reacts:
     * const [todos, setTodos] = useState<std::vector<Todo>>({});
     * but due to limitations within c++, useState is a macro that defines
     * member variables and setters to the component */
};
```
Learn more about components [here]() and `useState()` and reactivity [here]().

### Tailwind-Inspired Styling
Chainable modifiers make styling fact and readable, macros ensure expand in the pre-processor, ensuring no runtime-performance loss
```cpp
Div(
    w(200) h(100)        // Fixed width and height
    bg(45, 45, 45)       // Background colour
    p(10) m(5)           // Padding & margin
    rounded(8)           // Border radius
    centerXY             // Center content on both axis
)
```

### Flexibile Panel System
Declare layouts directly in source per-window that the user can or cannot modify with resizable splitters, inspired by foobar2000's ColumnsUI plugin.
```cpp
HSplitter() {
    Panel("Sidebar");
    VSplitter() {
        Panel("Editor");
        Panel("Console");
    }
}
```

### Advanced Text Rendering
Full Unicode support including CJK and greyscale emojis, multiple font styles, LCD subpixel rendering, and cached glyph atlases for performance.
```cpp
TextU32(U"Hello 世界", 
    fontSize(16) 
    textColour(255, 255, 255)
    justify_center);
```

## Current Status
SableUI is approaching v1.0. and the core features are stable, but no limited component library as of current so use cases are slim

When v1.0. is done, the following features will be fully implmented:
- Vulkan/Metal backends
- Tested linux & macOS support
- Component library (scrollviews, input fields, tab stacks, sliders, etc)
- Expanded documentation
- Shader transpilation across multiple graphics backends

## Platform Support
...

## Graphics Backends
- OpenGL 3.3+: 98% (small bug)
- Vulkan and Metal: coming soon

---

### Next steps:
<div class="card-grid">
  <div class="card" onclick="window.location.href='getting-started.html';" style="cursor: pointer;">
    <img src="path/to/image2.jpg" alt="Card image">
    <h2>Getting Started</h2>
    <p class="subtitle">Build a starter application in under 5 minutes</p>
  </div>

  <div class="card">
    <img src="path/to/image3.jpg" alt="Card image">
    <h2>Core concepts</h2>
    <p class="subtitle">Understand concepts such as components, states and more</p>
  </div>

  <div class="card">
    <img src="path/to/image2.jpg" alt="Card image">
    <h2>Examples</h2>
    <p class="subtitle">See real applications</p>
  </div>
</div>