# Styling Guide
SableUI uses a Tailwind-inspired styling system with chainable modifers. Styles are applied inline where elements are delcared, eliminating the need for verbose styling or seperate stylesheet files.

## Philosophy
Traditional UI frameowkrs seperate structure and style, but SableUI brings styles directly to your component definition, just like the powerful tailwind + react stack, without the overhead.

**Example:**
```cpp
Div(bg(90, 160, 255) p(12) rounded(8) w(120)) {
    Text("Click me", textColour(255, 255, 255) fontSize(16));
}
```

## Core Concepts
Modifiers (like `w(200)`) return references to a hidden ElementInfo object, allowing chaining in any order:
```cpp
Div(w(200) h(100) bg(45, 45, 45) p(10) m(5) rounded(8))
```
Each modifies is a macro that calls a setter method:
- `w(200)` → `.setWidth(200)`
- `bg(45, 45, 45)` → `.setBgColour(Colour(45, 45, 45))`

You can find a list of the modifers and definitions in `SableUI/SableUI.h` or in this table [here](style-reference.md).

### Spacing Units
All spacing values are in pixels. There are no relative units (%) currently and isn't planned anytime soon.
```cpp
Div(w(300) h(200))  // 300px wide, 200px tall
```
## Layout Properties
### Width & Height
```cpp
// Fixed dimensions
Div(w(200) h(100))

// Fill available space (shared evenly across other 'fill' siblings)
Div(w_fill h_fill)

// Fit content (default for most elements)
Div(w_fit h_fit)

// Constraints
Div(minW(100) maxW(500) minH(50) maxH(300))
```
### Width/Height Types:
- `w(value)`/`h(value)` - Fixed size in pixels
- `w_fill`/`h_fill` - Fill available space
- `w_fit`/`h_fit` - Fit to content size
- `minW/maxW/minH/maxH` - Size constrains in pixels
#### Example:
```cpp
Div(w_fill h_fit minH(200) maxH(600)) {
    // Takes full width, height fits content
    // but constrained between 200-600px
}
```
### Margin & Padding
```cpp
// All sides
Div(m(10) p(20))

// Horizontal/Vertical
Div(mx(15) my(10) px(20) py(15))

// Individual sides
Div(mt(5) mr(10) mb(5) ml(10))
Div(pt(5) pr(10) pb(5) pl(10))
```
#### Example:
```cpp
Div(bg(200, 200, 200) m(20) p(15)) {
    // 20px margin (transparent space outside)
    // 15px padding (gray space inside, before content)
    Text("Content");
}
```
### Layout Direction
Controls how child elements flow
```cpp
// Vertical (default)
Div(up_down) {      // Top to bottom
    Text("First");
    Text("Second");
}

Div(down_up) {      // Bottom to top
    Text("First");
    Text("Second");
}

// Horizontal
Div(left_right) {   // Left to right
    Text("First");
    Text("Second");
}

Div(right_left) {   // Right to left
    Text("First");
    Text("Second");
}
```
Default: `up_down` (top to bottom)
### Centering
```cpp
// Center horizontally
Div(centerX) {
    Text("Centered");
}

// Center vertically
Div(centerY) {
    Text("Centered");
}

// Center both axes
Div(centerXY) {
    Text("Centered");
}
```
**Note:** Centering applies to the element within its parent, not to the element's children.
#### Example
```cpp
Div(w(500) h(500) bg(255, 0, 0)) {
     // Rect is centered within its parent
    Rect(w(50) h(50) centerXY bg(0, 255, 0))
}
```
### Colours
The `bg` (background colour) modifier can use RGB or RGBA values:<br>
**Colour Format:** `bg(r, g, b, a)` where:
- `r`,`g`,`b` - Red, Green, Blue (0-255)
- `a` - Alhpa/opacity (0-255, default 255 (opaque))
```cpp
// Background color
Div(bg(45, 45, 45))           // RGB
Div(bg(45, 45, 45, 200))      // RGBA (with alpha)
```
The same rules go with the `textColour` property:
```cpp
// Text color
Text("Hello", textColour(255, 255, 255))
Text("Faded", textColour(200, 200, 200, 128))
```
And you can use the `rgb`/`rgba` modifier for added flexibility.<br>
Example with inline conditional colours:
```cpp
// Using rgb/rgba helpers
Div(bg(value == true ? rgb(255, 45, 45) : rgb(45, 255, 45)))
Div(bg(rgba(45, 45, 45, 200)))
```
### Border Radius
```cpp
// All corners
Div(rounded(8))

// Sharp corners
Div(rounded(0))

// Pill shape
Div(w(100) h(40) rounded(20))
```
Note: Individual corner radii are yet to be added. (v1.0).
## Text Properties
### Font Size
```cpp
Text("Small", fontSize(10))
Text("Normal", fontSize(14))
Text("Large", fontSize(24))
Text("Huge", fontSize(48))
```
**Default:** 11px
### Line Height
Controls spacing between lines of wrapped text:
```cpp
Text("Multi-line text that will wrap...",
    fontSize(14)
    lineHeight(1.5)  // 1.5x the font size
)
```
**Default:** 1.15 (15% taller than font size)
### Text Justification
```cpp
Text("Left aligned", justify_left)
Text("Centered", justify_center)
Text("Right aligned", justify_right)
```
**Default:** `justify_left`
### Text Wrapping
```cpp
// Wrap text (default)
Text("Long text that will wrap...", maxW(200) textWrap(true))

// No wrapping (minimum size is constrained to one line)
Text("Long text that won't wrap...", maxW(200) textWrap(false))
```
### Text Styling
Use [string methods](sablestring.md) for bold, italic, etc.

Learn more about how `SableString` functions and how why/how `.bold()` is implmented/required [here](sablestring.md).
#### Example with formatting:
```cpp
SableString message = 
    SableString("This is ") + 
    SableString("super cool").bold() + 
    SableString(" formatting").italic();
TextU32(message);
```
**Output:** This is **super cool** *formatting*

This will probably me improved with macros
### Absolute Positioning
```cpp
Div(absolute(100, 50)) {
    Text("At x:100, y:50");
}
```
> [!WARNING]
    Absolute positioning can mess up the layout tree significantly. For most use cases that require absolute positioning, it is recomended you use them with a [`CustomTargetQueue`](custom-render-targets.md) which is seperate to the default element tree. You can find out more about it in [Advanced Topics](custom-render-targets.md) and can view [documented implmentations](examples.md) of specific use cases like that require absolute positioning and [custom render targets](custom-render-targets.md) like [modals](modal.md).

<br>

---

<div class="card-grid">
  <div class="card" onclick="window.location.href='style-reference.html';" style="cursor: pointer;">
    <img src="path/to/image3.jpg" alt="Card image">
    <h2>Style Reference</h2>
    <p class="subtitle">Grid of definitions for style macros</p>
  </div>

  <div class="card">
    <img src="path/to/image3.jpg" alt="Card image">
    <h2>Event Handling</h2>
    <p class="subtitle">Add interactivity with onClick, onHover, etc.</p>
  </div>

  <div class="card">
    <img src="path/to/image2.jpg" alt="Card image">
    <h2>Examples</h2>
    <p class="subtitle">See real applications</p>
  </div>
</div>