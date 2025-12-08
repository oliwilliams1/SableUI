# Introduction
**SableUI** is a high-performance C++ UI framework that brings React's component model and Tailwind's styling approach to native applications - without the overhead of web technologies.

```cpp
class Counter : public SableUI::BaseComponent {
    void Layout() override {
        Div(bg(245, 245, 245) p(30) centerXY rounded(10)) {
            Text(SableString::Format("Count: %d", count),
                fontSize(28) mb(20) textColour(20, 20, 20));
            
            Div(onClick([=]() { setCount(count + 1); })) {
                Text("Increment");
            }
        }
    }
private:
    useState(count, setCount, int, 0);
};
```
<br>

---

<br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='what-is-sableui.html';" style="cursor: pointer;">
    <img src="path/to/image1.jpg" alt="Card image">
    <h2>What is SableUI?</h2>
    <p class="subtitle">Purpose and design philosophies</p>
  </div>

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
    <img src="path/to/image1.jpg" alt="Card image">
    <h2>API reference</h2>
    <p class="subtitle">Learn how to build a anything using SableUI's layout system</p>
  </div>

  <div class="card">
    <img src="path/to/image2.jpg" alt="Card image">
    <h2>Examples</h2>
    <p class="subtitle">See real applications</p>
  </div>

  <div class="card">
    <img src="path/to/image3.jpg" alt="Card image">
    <h2>Advanced topics</h2>
    <p class="subtitle">To build complex applications and access deeper APIs</p>
  </div>
</div>