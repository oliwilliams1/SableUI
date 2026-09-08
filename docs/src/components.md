# Components
SableUI ships a small component library on top of the core framework — enough to build the MVP without hand-rolling every button and text field. Each one is a normal `BaseComponent` under the hood, added via a macro that handles registering it as a scoped child and calling its `Init(...)`, the same pattern `Button(...)` uses in [Your First Application](your-first-application.md).

All of them read from the active theme (see [Theming](theming.md)) and respond to [Component Sizing](style-reference.md#component-sizing) (`size_sm`/`size_md`/`size_lg`/`size_none`, `disabled(...)`) unless noted otherwise.

## Button
```cpp
Button(label, callback, ...)
```
```cpp
Button("Save", [this]() { Save(); }, size_lg);
```
`label` accepts a `SableString`, so Unicode and emoji labels work directly — useful for icon-only buttons:
```cpp
Button(U"\U0001F4C5", [this]() { ToggleCalendar(); },
    w(16), h(16), fontSize(8), size_none, bg(rgba(0, 0, 0, 0)), rounded(999));
```
By default a button's background is `t.primary`, its corners are rounded 4px unless overridden, and its padding scales with `size_sm`/`size_md`/`size_lg` (or comes from your own `p`/`px`/`py` if you set one). `disabled(true)` both dims the button to `t.subtext0` and stops the callback from firing on click, so it's safe to leave a click handler wired up on a button that's conditionally disabled.

The button tracks its own pressed state internally (a `State<bool>` set from hit-testing in `OnUpdate`) purely to darken its background slightly while held — that state isn't exposed, so if you need to know whether a button is currently pressed from outside it, you'll need to track that yourself in the callback.

## Checkbox
Two macros, depending on who owns the boolean:
```cpp
// Externally-owned State<bool> — auto-syncs both ways
CheckboxState(label, checkedState, ...)

// Internally-owned bool with a callback
Checkbox(label, checked, onChange, ...)
```
```cpp
State<bool> agreed{ this, false };
// ...
CheckboxState("I agree to the terms", agreed);
```
```cpp
Checkbox("Enable notifications", notificationsEnabled,
    [this](bool v) { notificationsEnabled = v; });
```
Box size and label font size follow the same small/medium/large scale as `Button` (12px/15px/18px box, 10pt/11pt/13pt label). `disabled(...)` dims the checked colour rather than blocking the click outright — the click handler still checks `info.appearance.disabled` itself and no-ops, so a disabled checkbox won't toggle even though it's visually similar to an inactive button.

## TextField / InputField
```cpp
InputField(state, ...)  // single line
TextField(state, ...)   // multiline
```
Both take a `State<InputFieldData>&`, where `InputFieldData` holds the field's content, placeholder, focus state, and optional `onChange`/`onSubmit` callbacks:
```cpp
State<InputFieldData> name{ this, { .placeholder = "Your name" } };
// ...
InputField(name);
```
Typing, backspace/delete, arrow-key cursor movement with shift-to-select, and clipboard cut/copy/paste are all implemented already, and `onSubmit` fires on Enter for single-line fields (multiline inserts a newline instead). Clicking outside the field unfocuses it; Escape clears an active selection first, then unfocuses on a second press.

> [!WARNING]
    The visible text cursor and selection highlight aren't drawn yet — the underlying state (`cursorPos`, `cursorVisible`, a blinking `Interval`) is all tracked correctly, but the actual draw call in `OnUpdatePostLayout` is still commented out pending a custom render target. Typing and selecting both work; you just can't currently see the caret while doing it.

If you're building a component around a text field that needs extra content alongside it — an icon, a button, a suffix — override `ContentLeft()`/`ContentRight()` rather than reimplementing the field. This is how `DatePickerComponent` adds its calendar button without touching any of the text-editing logic.

## DatePicker
```cpp
DateField(state, ...)
```
`DatePickerComponent` is a `TextFieldComponent` subclass — it displays a formatted date and syncs it from an internal `CalendarContext`, and adds a calendar-icon button via `ContentRight()`:
```cpp
State<InputFieldData> deadline{ this, {} };
// ...
DateField(deadline);
```
> [!WARNING]
    The calendar icon button toggles the underlying "open" state correctly, but the floating panel that's meant to actually show the popup calendar (`CalendarHelperPostLayout`) is currently commented out, so clicking it doesn't yet show anything on screen. It also doesn't currently restrict typed input the way a finished date picker would — since it inherits full text editing from `TextFieldComponent`, typing directly into the field is still possible even though the intended flow is picking a date from the calendar.

If you need a working calendar today rather than the popup integration, use `Calendar` directly (below) — it's the piece that's actually finished, the popup wiring around it is what's still in progress.

## Calendar
`Calendar` can be embedded directly as an inline date picker, independent of `DatePicker`:
```cpp
State<CalendarContext> ctx{ this, {} };
// ...
ComponentScoped(calendar, Calendar, this)
{
    calendar->Init(ctx);
}
```
It initialises itself to today's date the first time it's laid out if `ctx` hasn't been set yet, renders a month header with prev/next navigation, and highlights the selected day. Reading the selection back out is just `ctx.get().selectedDay` / `.selectedMonth` / `.selectedYear`.

A few free functions handle the parts that don't need a live `Calendar` instance on screen — useful if you're driving the same `CalendarContext` from elsewhere in your UI:
```cpp
InitCalendarToToday(ctx);
InitCalendarToDate(ctx, 2026, 5, 1); // month is 0-indexed
```

<br><br>

---

<br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='state-management.html';" style="cursor: pointer;">
    <h2>State Management</h2>
    <p class="subtitle">State&lt;T&gt;, Ref&lt;T&gt;, Timer, and Interval</p>
  </div>
  <div class="card" onclick="window.location.href='theming.html';" style="cursor: pointer;">
    <h2>Theming</h2>
    <p class="subtitle">Where t.primary, t.surface0, and friends come from</p>
  </div>
  <div class="card" onclick="window.location.href='events.html';" style="cursor: pointer;">
    <h2>Event Handling</h2>
    <p class="subtitle">onClick, OnUpdate, and reading input state</p>
  </div>
</div>