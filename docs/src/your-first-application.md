# Building a Counter

This walks through one small, complete SableUI application, built up piece by piece. By the end it covers application setup, components, layout, styling, reactive state, interaction, composing components, and reacting to time.

### Setup

Create `main.cpp` and include the main SableUI header, along with the (optional) style namespace:

```cpp
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;
```

### The application lifecycle

Every SableUI application follows the same shape in `main()`:

```cpp
int main()
{
	InitialisePrimaryWindow();

	while (WaitEvents())
		Render();

	Shutdown();

	return 0;
}
```

`InitialisePrimaryWindow()` creates the window and sets up the renderer. `WaitEvents()` handles input and returns `false` once the window should close, so it doubles as the loop condition. `Render()` draws the current frame. `Shutdown()` cleans everything up. This shape doesn't change regardless of what the application actually does — application logic lives elsewhere, not in this loop.

> **Note:** `WaitEvents()` can be switched out with `WaitEventsTimeout(double timeout)` or `PollEvents()` based on application type. `WaitEvents()` is typically used for general applications that do not require updates to match the refresh rate of a display, `PollEvents()` loops instantanously, useful for games, and `WaitEventsTimeout(double timeout)` is a balance between the two, which waits for events or the specified time runs out.

### Your first component

A component is a class deriving from `BaseComponent` that overrides `Layout()`:

```cpp
class Counter : public BaseComponent
{
public:
	void Layout() override
	{
		Text("Count: 0");
	}
};
```

`Layout()` is called during rendering and is where the component's contents are declared — including any conditional logic or loops, since it's ordinary C++. `Text(...)` adds a text element to the layout.

For example, you can have the following code which lays out exactly as it reads, which is a luxery some solutions don't have, and is built right into the core of SableUI.

```cpp
class Counter : public BaseComponent
{
public:
	void Layout() override
	{
		for (int i = 0; i < 10; i++)
		{
			if (i % 2 == 0)
			{
				Text(SableString::Format("Element number: %d", i));
			}
		}
	}
};
```

But for the sake of this tutorial, we will revert back to the earlier snippet.

### Displaying it

Before the main loop, register the component under a name, then add a panel that uses it:

```cpp
int main()
{
	InitialisePrimaryWindow();
	RegisterComponent<Counter>("Counter");

	Panel("Counter");

	while (WaitEvents())
		Render();

	Shutdown();

	return 0;
}
```

`RegisterComponent<T>(string key)` makes `Counter` creatable by string elsewhere in the framework. `Panel("Counter")` adds a panel to the window's panel tree and attaches an instance of the registered component to it.

### Layout and styling

`Div(...) { ... }` groups elements together and applies styling to the group. It's a scoped construct — the braces aren't decorative, they define which elements are children of that div:

```cpp
void Layout() override
{
	const Theme& t = GetTheme();

	Div(bg(t.surface0), p(30), centerXY, rounded(10))
	{
		Text("Count: 0", fontSize(28), mb(20), textWrap(false));
	}
}
```

Each argument to `Div(...)` or `Text(...)` — `bg(...)`, `p(30)`, `centerXY`, `rounded(10)`, `fontSize(28)`, `mb(20)` — is a small style value. They can be freely mixed and chained in any order. `GetTheme()` returns the current theme, so colours can be pulled from it (`t.surface0`) rather than hardcoded, which keeps components reusable across different themes.

### Making the count reactive

To make `Count: 0` into a real counter, add a `State<int>` member:

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
		}
	}

private:
	State<int> count{ this, 0 };
};
```

`State<T>` takes the owning component (`this`) and a default value. Unlike a plain member variable, a `State<T>` survives across rerenders and reconciliation, and calling `.set(...)` on it automatically marks the component dirty and schedules a rerender — there's no separate step to tell the framework something changed. `.get()` reads the current value. `SableString::Format(...)` works like `sprintf`.

### Adding interactivity

`Button(label, callback, ...)` wires a click to a callback, and takes the same style arguments as `Div` or `Text`:

```cpp
Div(left_right, mb(8))
{
	Button("Increment", [this]() { count.set(count.get() + 1); }, mr(4));
	Button("Decrement", [this]() { count.set(count.get() - 1); });
}
```

`left_right` lays the div's children out horizontally instead of the default vertical stacking. The callback is a lambda capturing `this`, so it can reach back into the component's own state — clicking the button calls `count.set(...)`, which triggers the same rerender as any other state change.

### Composing components

A component can nest another component inside its `Layout()`. Here's a second component that just displays a string it's given:

```cpp
class IntervalDisplay : public BaseComponent
{
public:
	void Layout() override
	{
		Text("Child component");
		Text(formattedTime, textWrap(false));
	}

	void SetFormattedTime(const SableString& str)
	{
		formattedTime = str;
	}

private:
	SableString formattedTime = ""; // Not state!
};
```

`formattedTime` is a plain member, not a `State<T>`. It doesn't need to be state because it's recomputed and handed down fresh by the parent every time `Layout()` runs. If it were a state, and improper state would be preserved, as it will initialise the internal state after the component scope in the parent ends.

To use it inside `Counter`, register it and nest it with `ComponentScoped`:

```cpp
ComponentScoped(intervalDisplay, IntervalDisplay, this, bg(t.surface1), rounded(8), p(8))
{
	intervalDisplay->SetFormattedTime(
		SableString::Format("Seconds since application start: %d", time.get())
	);
}
```

`ComponentScoped(name, Type, owner, <optional> styleArgs) { ... }` creates a child component of the given type, gives back a pointer (`intervalDisplay`) usable inside the braces, and attaches it to `owner` once the block ends. Data flows one way here: `Counter` calls a setter on `IntervalDisplay`, and `IntervalDisplay` has no way to reach back into `Counter`'s state. The optional field: `styleArgs` is used for applying styling to the childs containing element.

### Reacting to time

`OnUpdate(const UIUpdateContext&)` is a separate override from `Layout()`, called every frame, for logic that reacts to time or input rather than declaring what's on screen. Combined with an `Interval`, it can drive state changes on a schedule:

```cpp
class Counter : public BaseComponent
{
public:
	Counter()
	{
		interval.Start(1000);
	}

	void OnUpdate(const UIUpdateContext& ctx) override
	{
		if (interval.IsFired(ctx))
		{
			time.set(time.get() + 1);
		}
	}

private:
	Interval interval{ this };
	State<int> time{ this, 0 };
};
```

`interval.Start(1000)` schedules the interval to fire every 1000ms, starting in the constructor. `interval.IsFired(ctx)` checks whether it fired this frame. Because `time.set(...)` marks the component dirty the same way `count.set(...)` did earlier, updating `time` from `OnUpdate` triggers a rerender through the exact same path a button click does — there's no separate mechanism to learn for time-driven versus user-driven updates.

### Putting it together

```cpp
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;

class IntervalDisplay : public BaseComponent
{
public:
	void Layout() override
	{
		Text("Child component");
		Text(formattedTime, textWrap(false));
	}

	void SetFormattedTime(const SableString& str)
	{
		formattedTime = str;
	}

private:
	SableString formattedTime = ""; // Not state!
};

class Counter : public BaseComponent
{
public:
	Counter()
	{
		interval.Start(1000);
	}

	void Layout() override
	{
		const Theme& t = GetTheme();

		Div(bg(t.surface0), p(30), centerXY, rounded(10))
		{
			Text(SableString::Format("Count: %d", count.get()), fontSize(28), mb(20), textWrap(false));

			Div(left_right, mb(8))
			{
				Button("Increment", [this]() { count.set(count.get() + 1); }, mr(4));
				Button("Decrement", [this]() { count.set(count.get() - 1); });
			}
			ComponentScoped(intervalDisplay, IntervalDisplay, this, bg(t.surface1), rounded(8), p(8))
			{
				intervalDisplay->SetFormattedTime(
					SableString::Format("Seconds since application start: %d", time.get())
				);
			}
		}
	}

	void OnUpdate(const UIUpdateContext& ctx) override
	{
		if (interval.IsFired(ctx))
		{
			time.set(time.get() + 1);
		}
	}

private:
	State<int> count{ this, 0 };

	Interval interval{ this };
	State<int> time{ this, 0 };
};

int main()
{
	InitialisePrimaryWindow();
	RegisterComponent<Counter>("Counter");
	RegisterComponent<IntervalDisplay>("IntervalDisplay");

	Panel("Counter");

	while (WaitEvents())
		Render();

	Shutdown();

	return 0;
}
```