# Event Guide
SableUI provides a safe event system that allows you to create interactive components with mouse, keyboard, and scroll input. Simple element events can be attached directly in the `Layout()` phase using inline callbacks; anything more involved — held keys, drag state, hit-testing — goes through the `OnUpdate()` method instead.

## Inline Events
SableUI provides a small set of callbacks that attach directly to an element, for the common cases where you just need "something happened to this specific element."

> Everything else — scrolling, all keyboard events, held/dragged state, and hover — is read from the `UIUpdateContext` in `OnUpdate()`, covered below.

### onClick
Triggered when the left mouse button is clicked on an element.
```cpp
Div(onClick([this]() {
    count.set(count.get() + 1);
    SableUI_Log("Clicked! Count: %d", count.get());
}))
{
    Text(SableString::Format("Click me, num clicks: %d", count.get()),
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
Commonly paired with a context menu shown as a floating panel positioned at the click location.
### onDoubleClick
Triggered when an element is clicked twice within a short window.
> **Note:** The double-click timing window is 300ms, and clicks must be within 5 pixels of each other to register as a double-click. These thresholds are constants on the `Window` class in `window.h` — they aren't currently exposed as something an application can configure per-instance.

> [!WARNING]
    State lambdas can be dangerous and cause problems if used incorrectly — reference-capturing lambdas (`[&]`) can be unstable if the referenced variable goes out of scope before the callback fires. The best practice is to capture `this` and other arguments by **value**, for example: `onClick([this, otherVar1, otherVar2]() {});`.

### Hover
There's no `onHover`/`onHoverExit` inline callback yet — `ElementInfo` doesn't carry hover callbacks the way it does `onClickFunc`. Until that lands, hover has to be computed manually in `OnUpdate()` by hit-testing the mouse position against an element's rect, the same way `ButtonComponent` tracks its own pressed state internally:
```cpp
void OnUpdate(const UIUpdateContext& ctx) override
{
    Element* root = GetRootElement();
    if (!root) return;

    bool hovered = RectBoundingBox(root->rect, ctx.input.mousePos, ctx.input.obscurers, ctx.zIndex);
    isHovered.set(hovered);
}
```

---

## Keyboard Input
Keyboard events are global rather than element-specific, so there's no `onKeyPress(...)` element callback — instead, override `OnUpdate()` and read key state off the input context directly. If a keyboard shortcut should only fire while the component is hovered, pair it with the same `RectBoundingBox` hit-test shown above.

### Accessing the Event Context
Override `OnUpdate()` in your component to reach keyboard, mouse, and timer state:
```cpp
class MyComponent : public SableUI::BaseComponent {
public:
    void Layout() override {
        // Your UI layout here
    }

    void OnUpdate(const UIUpdateContext& ctx) override {
        // ctx.input is the UIInputState for this frame
        // ctx.zIndex is this component's current z-index
    }
};
```

### Key Constants
SableUI provides `constexpr` constants for all keyboard keys, borrowed from [`GLFW`](https://www.glfw.org) (the window manager) for easy translation. These constants follow the pattern `SABLE_KEY_*`:

> A list of these keys can be grabbed from [`events.h`](https://github.com/oliwilliams1/SableUI/blob/master/include/SableUI/core/events.h)

### Key State Queries
`ctx.input` provides three ways to query key states, each a `std::bitset<SABLE_MAX_KEYS>`:

#### isKeyDown
`true` every frame while the key is held down.
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    if (ctx.input.isKeyDown.test(SABLE_KEY_W))
    {
        // Move forward continuously
        posY.set(posY.get() - speed * ctx.input.deltaTime);
    }

    if (ctx.input.isKeyDown.test(SABLE_KEY_S))
    {
        // Move backward continuously
        posY.set(posY.get() + speed * ctx.input.deltaTime);
    }
}
```
#### keyPressedEvent
`true` only on the frame a key is pressed. Use for single actions.
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    if (ctx.input.keyPressedEvent.test(SABLE_KEY_SPACE))
    {
        // Toggle state once per press
        isPaused.set(!isPaused.get());
    }
}
```
#### keyReleasedEvent
`true` only on the frame a key is released.
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    if (ctx.input.keyReleasedEvent.test(SABLE_KEY_LEFT_SHIFT))
    {
        // Stop running when shift is released
        isRunning.set(false);
    }
}
```

#### Modifier Keys
These tests can be combined to build key combination events.
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    bool ctrlPressed = ctx.input.isKeyDown.test(SABLE_KEY_LEFT_CONTROL) ||
                        ctx.input.isKeyDown.test(SABLE_KEY_RIGHT_CONTROL);

    // Ctrl+S for save
    if (ctrlPressed && ctx.input.keyPressedEvent.test(SABLE_KEY_S))
    {
        Save();
    }
}
```
Text input itself — actual typed characters, as opposed to individual key presses — comes through `ctx.input.typedCharBuffer`, a `std::vector<unsigned int>` of codepoints typed this frame. See `TextFieldComponent` in [Components](components.md#textfield--inputfield) for a full example handling typed input, selection, and clipboard together.

## Mouse Position and Scrolling
`ctx.input` also carries mouse position and scroll information:
### Mouse Position
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    int mouseX = ctx.input.mousePos.x;
    int mouseY = ctx.input.mousePos.y;

    // Mouse delta since last frame
    int deltaX = ctx.input.mouseDelta.x;
    int deltaY = ctx.input.mouseDelta.y;

    SableUI_Log("Mouse pos: %dx%d, mouse delta: %dx%d",
        mouseX, mouseY, deltaX, deltaY);
}
```
### Scrolling
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    float scrollX = ctx.input.scrollDelta.x;
    float scrollY = ctx.input.scrollDelta.y;

    if (scrollY != 0.0f)
    {
        // Zoom in/out based on scroll
        zoomLevel.set(zoomLevel.get() + scrollY * 0.1f);
    }
}
```
### Mouse Button State
Mouse buttons are queried the same way as keyboard keys, against `SABLE_MOUSE_BUTTON_*` constants:
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    // Check if left mouse button is held down
    if (ctx.input.mouseDown.test(SABLE_MOUSE_BUTTON_LEFT))
    {
        // Drag operation
        dragX.set(dragX.get() + ctx.input.mouseDelta.x);
        dragY.set(dragY.get() + ctx.input.mouseDelta.y);
    }

    // Check for mouse button press
    if (ctx.input.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
    {
        isDragging.set(true);
    }

    // Check for mouse button release
    if (ctx.input.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
    {
        isDragging.set(false);
    }
}
```
There's also `mouseDoubleClicked`, tested the same way, if `onDoubleClick` on a specific element isn't granular enough for what you need.

## Delta Time
`ctx.input.deltaTime` gives frame-independent timing for animation and movement:
```cpp
void OnUpdate(const UIUpdateContext& ctx) override {
    if (ctx.input.isKeyDown.test(SABLE_KEY_RIGHT))
    {
        // Move at constant speed regardless of frame rate
        posX.set(posX.get() + speed * ctx.input.deltaTime);
    }
}
```
`deltaTime` is in seconds, so if `speed = 100.0f`, the object moves at 100 pixels per second.

## Timers
For anything on a schedule rather than tied to a specific input — a blinking cursor, a polling interval, a delayed action — use `Timer` or `Interval` instead of checking `deltaTime` by hand. See [State Management](state-management.md#timer-and-interval) for how those work and how they interact with `OnUpdate`.

<br><br>

---

<br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='state-management.html';" style="cursor: pointer;">
    <h2>State Management</h2>
    <p class="subtitle">State&lt;T&gt;, Ref&lt;T&gt;, Timer, and Interval for reactive components</p>
  </div>
  <div class="card" onclick="window.location.href='components.html';" style="cursor: pointer;">
    <h2>Components</h2>
    <p class="subtitle">Button, Checkbox, TextField, DatePicker, and Calendar</p>
  </div>
  <div class="card">
    <h2>Examples</h2>
    <p class="subtitle">See event handling in real applications</p>
  </div>
</div>