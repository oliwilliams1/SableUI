# State Management
Every SableUI component can rebuild its `Layout()` at any point, so any value that needs to survive that rebuild — and any value that should *cause* a rebuild when it changes — has to be declared as one of SableUI's state types rather than a plain member variable. There are four of them: `State<T>`, `Ref<T>`, `Timer`, and `Interval`.

All four are declared as members and constructed with the owning component as their first argument, which registers them so the framework can carry their value across reconciliation:
```cpp
class MyComponent : public BaseComponent
{
private:
    State<int> count{ this, 0 };
};
```
A plain member variable (no state wrapper) is still completely valid — it's the right choice for anything recomputed fresh every `Layout()` call, such as a value handed down from a parent. See the `formattedTime` member in [Your First Application](your-first-application.md) for that pattern. State types exist for the values that don't fit that description.

## State\<T\>
`State<T>` is the one you'll reach for by default — a value the component owns, that persists across rerenders, and that triggers a rerender when it changes.
```cpp
State<int> count{ this, 0 };
```
Read it with `.get()`, and change it with `.set(...)`:
```cpp
Text(SableString::Format("Count: %d", count.get()));
// ...
count.set(count.get() + 1);
```
`.set(...)` checks the new value against the current one first — if they're equal, nothing happens; if they differ, the value is updated and the owning component is marked dirty for a rerender. This means calling `.set()` with an unchanged value is cheap and safe to do unconditionally, and it's also why `T` has to support `operator==` — `State<T>` won't compile for a type that can't be compared.

`State<T>` also supports assignment and implicit conversion, so `count = count + 1;` and `int c = count;` both work, though `.get()`/`.set()` are the clearer choice in most code.

## Ref\<T\>
`Ref<T>` looks almost identical to `State<T>` — same constructor shape, same registration with the owner — but `.set(...)` does **not** mark the component dirty. It exists for values that need to survive across rerenders (so they can't just be a plain member) but shouldn't themselves cause a rerender when they change.

The clearest real example is a stored callback. `Checkbox` keeps the caller's `onChange` handler in a `Ref`, not a `State`:
```cpp
Ref<std::function<void(bool)>> onChangeCallback{ this, nullptr };
```
Wrapping that in `State<T>` instead would be actively wrong here — `std::function` doesn't have `operator==`, so it wouldn't compile, and even if it did, reassigning a callback is not something that should trigger a visual rerender on its own.

As a rule: if changing the value should update what's on screen, use `State<T>`. If it just needs to persist and doesn't affect layout directly, use `Ref<T>`.

## Timer and Interval
Both `Timer` and `Interval` hook into the background event scheduler rather than driving a value directly — they're for scheduling *when* something happens, and you still update a `State<T>` yourself once it does.

`Timer` fires once, after a delay:
```cpp
Timer saveDelay{ this };
// ...
saveDelay.Start(2000); // fires once, 2000ms from now
```
`Interval` fires repeatedly, on a fixed period:
```cpp
Interval tick{ this };
// ...
tick.Start(1000); // fires every 1000ms
```
Both are checked from `OnUpdate()`. `Interval` has a convenience method for this:
```cpp
void OnUpdate(const UIUpdateContext& ctx) override
{
    if (tick.IsFired(ctx))
    {
        seconds.set(seconds.get() + 1);
    }
}
```
`Timer` doesn't expose its own `IsFired`, so check it against the input context directly using its handle — this form also works for `Interval`, and is exactly what `Interval::IsFired(ctx)` does internally:
```cpp
void OnUpdate(const UIUpdateContext& ctx) override
{
    if (ctx.input.IsFired(saveDelay.GetHandle()))
    {
        DoSave();
    }
}
```
Both also expose `.Stop()` and `.Reset()` — `Reset()` restarts the countdown/period from now without needing to call `.Start()` again with the same duration, which is how `TextFieldComponent` keeps its cursor blinking on a steady rhythm restarted every time a key is pressed, rather than drifting.

<br><br>

---

<br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='events.html';" style="cursor: pointer;">
    <h2>Event Handling</h2>
    <p class="subtitle">Add interactivity with onClick, OnUpdate, and input state</p>
  </div>
  <div class="card" onclick="window.location.href='components.html';" style="cursor: pointer;">
    <h2>Components</h2>
    <p class="subtitle">Button, Checkbox, TextField, DatePicker, and Calendar</p>
  </div>
  <div class="card">
    <h2>Examples</h2>
    <p class="subtitle">See state management in real applications</p>
  </div>
</div>