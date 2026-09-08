# Introduction
**SableUI** is a high-performance C++ UI framework that brings React's component model and Tailwind's styling approach to native applications - without the overhead of web technologies.

```cpp
class Counter : public BaseComponent
{
public:
	void Layout() override
	{
		const Theme& t = GetTheme();

		Div(bg(t.surface0), p(30), centerXY, rounded(10))
		{
			Text(SableString::Format("Count: %d", count.get()), fontSize(28), mb(20), textWrap(false));

			Div(left_right)
			{
				Button("Increment", [this]() { count.set(count.get() + 1); }, mr(4));
				Button("Decrement", [this]() { count.set(count.get() - 1); });
			}
		}
	}

private:
	State<int> count{ this, 0 };
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