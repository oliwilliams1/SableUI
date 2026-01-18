# Event Guide
SableUI provides a safe event system that allows you to create interactive components with mouse, keyboard, and scroll input. Events can be attached directly to elements in the `Layout()` phase using inline callbacks, making it easy to build reactive UIs without complex custom even propagation logic.

## Inline Events
SableUI provides simple callbacks that are element-specific, perfect for the common use cases.

> Other events such as scrolling, all keyboard events, and non-standard mouse events are packaged alone with the `UIEventContext`.

### onClick
Triggered when the left mouse button is clicked on an element.
```cpp
Div(onClick([this]() {
    setCount(count + 1);
    SableUI_Log("Clicked! Count: %d", count);
}))
{
    Text(SableString::Fomrat("Click me, num clicks: %d", count),
        textColour(255, 255, 255));
}
```
### onSecondaryClick
Triggered when the right mouse button is clicked on an element.
```cpp
Div(onSecondaryClick([this]() {
    SableUI_Info("Right clicked");
}))
{
    Text("Right-click me", textColour(200, 200, 200));
}
```
// TODO
Can be paired alongside a context menu like this:
### onDoubleClick
Triggered when an element is double-clicked within a short time window within an element.
> **Note:** The double-click timing window is 300ms, and clicks must be within 5 pixels of each other to register as a double-click. Can be modified in `window.h`.

### onHover & onHoverExit
Triggered when the mouse pointer enters and exits the bounds of an element respectively.
```cpp
Div(onHover([this]() {
    setIsHovered(true);
}) onHoverExit([this]() {
    setIsHovered(false);
}))
{
    Text("Hover me", textColour(isHovered ? 100 : 200, 200, 200));
}
```

> [!WARNING]
    State lambas can be dangerous and cause problems if used incorrectly, for example, reference lambdas (`[&]`) can be unstable if used wrong. The best practice is to capture `this` and other arguments by **value**, for example: `onClick([this, otherVar1, otherVar2]() {});`.

---

## Keyboard Input
For keyboard input, SableUI provides access to the global event context through the `OnUpdate()` method.

> Unlike mouse events which are element-specific, keyboard events are global, so it can be paired with `RectBoundingBox(Rect r, ivec2 p)` where `r` can be the rect of `rootElement` and `p` can be `ctx.mousePos` for only registering key presses when the cursor is hovering your component for example.

### Accessing the Event Context
Override the `OnUpdate()` method in your component to access keyboard or other states:
```cpp
class MyComponent : public SableUI::BaseComponent {
public:
    void Layout() override {
        // Your UI layout here
    }

    void OnUpdate(const UIEventContext& ctx) override {
        // Handle keyboard input here
    }
};
```

### Key Constants
SableUI provides `constexpr` constants for all keyboard keys, borrowed from [`GLFW`](https://www.glfw.org) (the window manager) for easy translation. These constants follow the pattern `SABLE_KEY_*`:

> A list of these keys can be grabbed in the file [`events.h`](https://github.com/oliwilliams1/SableUI/blob/master/include/SableUI/events.h)

### Key State Queries
The `UIEventContext` provides three ways to query key states:

#### isKeyDown
Returns `true` every frame while the key is held down.
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    if (ctx.isKeyDown.test(SABLE_KEY_W)) {
        // Move forward continuously
        posY -= speed * ctx.deltaTime;
        MarkDirty();
    }
    
    if (ctx.isKeyDown.test(SABLE_KEY_S)) {
        // Move backward continuously
        posY += speed * ctx.deltaTime;
        MarkDirty();
    }
}
```
#### keyPressedEvent
Returns `true` only on the frame a key is pressed. Use for single actions.
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    if (ctx.keyPressedEvent.test(SABLE_KEY_SPACE)) {
        // Toggle state once per press
        setIsPaused(!isPaused);
    }
}
```
#### keyReleasedEvent
Returns `teu` only on the frame when a key is released.
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    if (ctx.keyReleasedEvent.test(SABLE_KEY_LEFT_SHIFT)) {
        // Stop running when shift is released
        setIsRunning(false);
    }
}
```

#### Modifer Keys
These tests can be paird amongst others to create key combination events.
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    bool ctrlPressed = ctx.isKeyDown.test(SABLE_KEY_LEFT_CONTROL) || 
                       ctx.isKeyDown.test(SABLE_KEY_RIGHT_CONTROL);
    
    // Ctrl+S for save
    if (ctrlPressed && ctx.keyPressedEvent.test(SABLE_KEY_S)) {
        Save();
    }
}
```

## Mouse Position and Scrolling
The `UIEventContext` also provides mouse position and scroll information:
### Mouse Position
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    int mouseX = ctx.mousePos.x;
    int mouseY = ctx.mousePos.y;
    
    // Mouse delta since last frame
    int deltaX = ctx.mouseDelta.x;
    int deltaY = ctx.mouseDelta.y;
    
    SableUI_Log("Mouse pos: %dx%d, mouse delta: %dx%d",
        mouseX, mouseY, deltaX, deltaY);
}
```
### Scrolling
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    float scrollX = ctx.scrollDelta.x;
    float scrollY = ctx.scrollDelta.y;
    
    if (scrollY != 0.0f) {
        // Zoom in/out based on scroll
        float zoomDelta = scrollY * 0.1f;
        setZoomLevel(zoomLevel + zoomDelta);
    }
}
```
### Mouse Button State
Query mouse button states similarly to keyboard keys:
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    // Check if left mouse button is held down
    if (ctx.mouseDown.test(SABLE_MOUSE_BUTTON_LEFT)) {
        // Drag operation
        dragX += ctx.mouseDelta.x;
        dragY += ctx.mouseDelta.y;
        needsRerender = true;
    }
    
    // Check for mouse button press
    if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT)) {
        // Start drag
        isDragging = true;
    }
    
    // Check for mouse button release
    if (ctx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT)) {
        // End drag
        setIsDragging(false);
    }
}
```
## Delta Time
The `UIEventContext` procides `deltaTime` for frame-independent animations and movement:
```cpp
void OnUpdate(const UIEventContext& ctx) override {
    if (ctx.isKeyDown.test(SABLE_KEY_RIGHT)) {
        // Move at constant speed regardless of frame rate
        posX += speed * ctx.deltaTime;
        needsRerender = true;
    }
}
```
`deltaTime` is in seconds, so if `speed = 100.0f`, the object moves at 100 pixels per second.

<br><br>

---

<br>
<div class="card-grid">
  <div class="card">
    <h2>State Management</h2>
    <p class="subtitle">Learn useState and useRef for reactive components</p>
  </div>
  <div class="card">
    <h2>Components</h2>
    <p class="subtitle">Understanding Layout(), OnUpdate(), and rendering</p>
  </div>
  <div class="card">
    <h2>Examples</h2>
    <p class="subtitle">See event handling in real applications</p>
  </div>
</div>