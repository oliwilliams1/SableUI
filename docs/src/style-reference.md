## Style Macros Reference

### Sizing
| Macro | Description | Example |
|-------|-------------|---------|
| `w(n)` | Fixed width | `w(200)` |
| `h(n)` | Fixed height | `h(100)` |
| `w_fill` | Fill parent width | `w_fill` |
| `h_fill` | Fill parent height | `h_fill` |
| `w_fit` | Fit content width | `w_fit` |
| `h_fit` | Fit content height | `h_fit` |
| `minW(n)` | Minimum width | `minW(100)` |
| `maxW(n)` | Maximum width | `maxW(500)` |
| `minH(n)` | Minimum height | `minH(50)` |
| `maxH(n)` | Maximum height | `maxH(300)` |

[Futher reference](styling-guide.md#width--height) for sizing styling

### Spacing
| Macro | Description | Example |
|-------|-------------|---------|
| `m(n)` | Margin (all sides) | `m(10)` |
| `mx(n)` | Margin horizontal | `mx(15)` |
| `my(n)` | Margin vertical | `my(10)` |
| `mt/mr/mb/ml(n)` | Individual margins | `mt(5)` |
| `p(n)` | Padding (all sides) | `p(20)` |
| `px(n)` | Padding horizontal | `px(15)` |
| `py(n)` | Padding vertical | `py(10)` |
| `pt/pr/pb/pl(n)` | Individual padding | `pt(5)` |

[Futher reference](styling-guide.md#margin--padding) for margin and padding styling

### Colors
| Macro | Description | Example |
|-------|-------------|---------|
| `bg(r,g,b)` | Background color | `bg(45,45,45)` |
| `bg(r,g,b,a)` | Background with alpha | `bg(45,45,45,200)` |
| `textColour(r,g,b)` | Text color | `textColour(255,255,255)` |
| `rgb(r,g,b)` | Color helper | `bg(rgb(45,45,45))` |
| `rgba(r,g,b,a)` | RGBA helper | `bg(rgba(45,45,45,200))` |

[Futher reference](styling-guide.md#colours) for colour styling

### Layout
| Macro | Description | Example |
|-------|-------------|---------|
| `up_down` | Children top→bottom | `up_down` |
| `down_up` | Children bottom→top | `down_up` |
| `left_right` | Children left→right | `left_right` |
| `right_left` | Children right→left | `right_left` |
| `centerX` | Center horizontally | `centerX` |
| `centerY` | Center vertically | `centerY` |
| `centerXY` | Center both axes | `centerXY` |

[Futher reference](styling-guide.md#layout-direction) for layout directions and centering

### Text
| Macro | Description | Example |
|-------|-------------|---------|
| `fontSize(n)` | Font size in pixels | `fontSize(16)` |
| `lineHeight(n)` | Line height multiplier | `lineHeight(1.5)` |
| `justify_left` | Align text left | `justify_left` |
| `justify_center` | Center text | `justify_center` |
| `justify_right` | Align text right | `justify_right` |
| `textWrap(bool)` | Enable/disable wrapping | `textWrap(false)` |

[Futher reference](styling-guide.md#text-properties) for text-based properties

### Visual
| Macro | Description | Example |
|-------|-------------|---------|
| `rounded(n)` | Border radius | `rounded(8)` |
| `absolutePos(x,y)` | Absolute position | `absolute(100,50)` |

> [!WARNING]
    Absolute positioning can mess up the layout tree significantly. For most use cases that require absolute positioning, it is recomended you use them with a [`CustomTargetQueue`](custom-render-targets.md) which is seperate to the default element tree. You can find out more about it in [Advanced Topics](custom-render-targets.md) and can view [documented implmentations](examples.md) of specific use cases like that require absolute positioning and [custom render targets](custom-render-targets.md) like [modals](modal.md).

<br>

---

<br><br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='styling-guide.html';" style="cursor: pointer;">
    <img src="path/to/image3.jpg" alt="Card image">
    <h2>Styling Guide</h2>
    <p class="subtitle">In-depth guide for all style modifiers</p>
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