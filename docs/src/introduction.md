# Introduction
Welcome to **SableUI** - a modern, high-performance C++ UI/application framework that takes the good from web-based applications without the negative performance impact and removes the bridge between the frontend and the backend for low-level applications.

## Who Is This For?
SableUI is designed for C++ developers who want want to write applications without the limitations of the web, featuring:
- Declarative, reactive components
- Cross-platform support
- Full control and open access over rendering and memory management

## Philosophy
Traditional C++ UI frameworks are often verbose and fatiguing with large limitations. Web frameworks are often the solution to this, but come with a performance cost and a large abstraction between UI and application. SableUI attempts to bridge this gap by bringing the developer experience of React to C++ while maintaining the speed and control you would get with a native C++ framework.
```cpp
// An example of a counter component
class Counter : public SableUI::BaseComponent {
public:
	void Layout() override {
		Div(bg(245, 245, 245) p(30) centerXY w_fit h_fit rounded(10))
        {
			Text(SableString::Format("Count: %d", count),
				fontSize(28) mb(20) textColour(20, 20, 20) justify_center);

			Div(left_right p(4) centerX rounded(9))
            {
				Div(bg(90, 160, 255) p(8) mr(5) rounded(5)
					onClick([=]() { setCount(count + 1); }))
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
		}
	}

private:
	useState(count, setCount, int, 0);
};
```

## Current Status
SableUI is incomplete but actively being developed and approaching v1.0. The core features are stable and production-ready, but won't get most too far, with ongoing work on:
- Vulkan/Metal support
- ID-based components for improved flexibility
- Expansion of component library (scrollable views, input fields, tab stacks, ...)
- Documentation and examples
- Panel docking (maybe not v1.0.)

---

Want to build something? [Quick Start](Quick-Start.md) or [Installation](Installation.md)