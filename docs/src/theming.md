# Theming
Most of SableUI's built-in components, and most of the styling examples elsewhere in these docs, pull their colours from the active theme rather than hardcoding RGB values:
```cpp
const Theme& t = GetTheme();

Div(bg(t.surface0), rounded(10))
{
    Text("Themed panel", textColour(t.text));
}
```
`GetTheme()` is a free function that returns the currently active `Theme` — a plain struct of `Colour` fields, so `t.surface0` is just a colour value like any you'd pass to `bg(...)` directly.

## Theme Structure
A `Theme` is organised into a few groups:

**Background layers** — `base`, `mantle`, `crust` — for the window background and progressively darker/recessed panels.

**Surfaces** — `surface0`, `surface1`, `surface2` — for elements that sit visually above the background, each more "elevated" than the last.

**Overlays** — `overlay0`, `overlay1`, `overlay2` — for borders, dividers, and other low-emphasis chrome, darkest to lightest.

**Text** — `subtext0` and `subtext1` for muted/secondary text, `text` for primary text — plus `subtext0Contrast`, `subtext1Contrast`, and `textContrast`, the same three but chosen to stay legible on top of an accent colour rather than the theme's background.

**Accent palette** — `rosewater`, `flamingo`, `pink`, `mauve`, `red`, `maroon`, `peach`, `yellow`, `green`, `teal`, `sky`, `sapphire`, `blue`, `lavender`. If those names look familiar, they're the same set used by the Catppuccin colour scheme.

**Semantic colours** — `primary`, `secondary`, `error`, `warning`, `success`, `info`, `checkColour` — the ones most component code actually reads (`ButtonComponent` uses `t.primary` for its default background, for instance). If a theme doesn't set these explicitly, they fall back to a colour from the accent palette (`primary`→`blue`, `error`→`red`, `warning`→`yellow`, `success`→`green`, `info`→`sky`, `checkColour`→`lavender`, `secondary`→`rosewater`) the first time `InitialiseSemantics()` runs. Both built-in themes override most of these with their own values anyway, so treat the fallback as a safety net for custom themes rather than something to rely on.

## Built-in Themes
SableUI registers two themes automatically: `"sableui_dark"` (the default active theme) and `"sableui_light"`. Switch between them with:
```cpp
ThemeManager::GetInstance().SetActiveTheme("sableui_light");
```
`SetActiveTheme` returns `false` and logs an error if the name isn't registered, rather than silently doing nothing.

## Registering a Custom Theme
A full theme is just a `Theme` struct populated field by field, registered under a name:
```cpp
Theme myTheme;
myTheme.name = "my_theme";
myTheme.base = Colour{ 18, 18, 20, 255 };
// ... fill in the rest ...
myTheme.InitialiseSemantics(); // fill any unset semantic colours from the palette

ThemeManager::GetInstance().RegisterTheme("my_theme", myTheme);
```

## Theme Variants
If you only want to tweak a few colours from an existing theme rather than define a whole new one, `ThemeOverride` lets you register a variant instead. Every field is `std::optional`, and only the ones you set are applied on top of the base theme:
```cpp
ThemeOverride highContrast;
highContrast.text = Colour{ 255, 255, 255, 255 };
highContrast.overlay1 = Colour{ 140, 140, 140, 255 };

ThemeManager::GetInstance().RegisterThemeVariant(
    "sableui_dark_high_contrast", "sableui_dark", highContrast
);
```
This resolves to a full `Theme` at registration time (`ThemeOverride::Apply`), so switching to the variant afterwards is exactly as cheap as switching to any other theme.

<br><br>

---

<br>
<div class="card-grid">
  <div class="card" onclick="window.location.href='styling-guide.html';" style="cursor: pointer;">
    <h2>Styling Guide</h2>
    <p class="subtitle">bg, textColour, and the rest of the style macros</p>
  </div>
  <div class="card" onclick="window.location.href='components.html';" style="cursor: pointer;">
    <h2>Components</h2>
    <p class="subtitle">Where t.primary, t.surface0, and friends actually get used</p>
  </div>
  <div class="card">
    <h2>Examples</h2>
    <p class="subtitle">See custom themes in real applications</p>
  </div>
</div>