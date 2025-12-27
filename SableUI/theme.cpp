// theme.cpp
#include <SableUI/theme.h>
#include <SableUI/console.h>

using namespace SableUI;

Theme ThemeOverride::Apply(const Theme& baseTheme) const
{
    Theme result = baseTheme;

#define APPLY_COLOR(field) if (field.has_value()) result.field = field.value()

    APPLY_COLOR(base);
    APPLY_COLOR(mantle);
    APPLY_COLOR(crust);
    APPLY_COLOR(surface0);
    APPLY_COLOR(surface1);
    APPLY_COLOR(surface2);
    APPLY_COLOR(overlay0);
    APPLY_COLOR(overlay1);
    APPLY_COLOR(overlay2);
    APPLY_COLOR(subtext0);
    APPLY_COLOR(subtext1);
    APPLY_COLOR(text);

    APPLY_COLOR(rosewater);
    APPLY_COLOR(flamingo);
    APPLY_COLOR(pink);
    APPLY_COLOR(mauve);
    APPLY_COLOR(red);
    APPLY_COLOR(maroon);
    APPLY_COLOR(peach);
    APPLY_COLOR(yellow);
    APPLY_COLOR(green);
    APPLY_COLOR(teal);
    APPLY_COLOR(sky);
    APPLY_COLOR(sapphire);
    APPLY_COLOR(blue);
    APPLY_COLOR(lavender);

    APPLY_COLOR(primary);
    APPLY_COLOR(secondary);
    APPLY_COLOR(error);
    APPLY_COLOR(warning);
    APPLY_COLOR(success);
    APPLY_COLOR(info);

#undef APPLY_COLOR

    return result;
}

ThemeManager& ThemeManager::GetInstance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
{
    RegisterTheme("sableui_dark", BuiltInThemes::SableUIDark());
}

void ThemeManager::RegisterTheme(const std::string& name, const Theme& theme)
{
    m_themes[name] = theme;
}

void ThemeManager::RegisterThemeVariant(const std::string& name,
    const std::string& baseName,
    const ThemeOverride & override)
{
    const Theme* base = GetTheme(baseName);
    if (!base)
    {
        SableUI_Error("Base theme '%s' not found", baseName.c_str());
        return;
    }

    m_themes[name] = override.Apply(*base);
}

bool ThemeManager::SetActiveTheme(const std::string& name)
{
    if (m_themes.find(name) == m_themes.end())
    {
        SableUI_Error("Theme '%s' not found", name.c_str());
        return false;
    }

    m_activeThemeName = name;
    SableUI_Log("Switched to theme: %s", name.c_str());
    return true;
}

const Theme& ThemeManager::GetActiveTheme() const
{
    auto it = m_themes.find(m_activeThemeName);
    if (it == m_themes.end())
    {
        return m_themes.at("sableui_dark");
    }
    return it->second;
}

const Theme* ThemeManager::GetTheme(const std::string& name) const
{
    auto it = m_themes.find(name);
    if (it == m_themes.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> ThemeManager::GetThemeNames() const
{
    std::vector<std::string> names;
    names.reserve(m_themes.size());
    for (const auto& pair : m_themes)
    {
        names.push_back(pair.first);
    }
    return names;
}

Theme SableUI::BuiltInThemes::SableUIDark()
{
    Theme t;
    t.name = "SableUI Dark";
    t.description = "Original SableUI dark theme";
    t.isDark = true;

    t.base      = Colour{  25,  25,  25, 255 };
    t.mantle    = Colour{  23,  23,  23, 255 };
    t.crust     = Colour{  20,  20,  20, 255 };

    t.surface0  = Colour{  38,  38,  38, 255 };
    t.surface1  = Colour{  43,  43,  43, 255 };
    t.surface2  = Colour{  51,  51,  51, 255 };

    t.overlay0  = Colour{  58,  58,  58, 255 };
    t.overlay1  = Colour{  78,  78,  78, 255 };
    t.overlay2  = Colour{ 104, 104, 104, 255 };

    t.subtext0  = Colour{ 135, 135, 135, 255 };
    t.subtext1  = Colour{ 175, 175, 175, 255 };
    t.text      = Colour{ 228, 228, 228, 255 };

    t.rosewater = Colour{ 255, 180, 180, 255 };
    t.flamingo  = Colour{ 255, 150, 150, 255 };
    t.pink      = Colour{ 255, 120, 200, 255 };
    t.mauve     = Colour{ 180, 120, 255, 255 };
    t.red       = Colour{ 220,  60,  60, 255 };
    t.maroon    = Colour{ 200,  50,  80, 255 };
    t.peach     = Colour{ 255, 180,  50, 255 };
    t.yellow    = Colour{ 255, 220, 100, 255 };
    t.green     = Colour{  60, 200, 100, 255 };
    t.teal      = Colour{  60, 200, 180, 255 };
    t.sky       = Colour{ 120, 200, 255, 255 };
    t.sapphire  = Colour{  80, 180, 255, 255 };
    t.blue      = Colour{  90, 160, 255, 255 };
    t.lavender  = Colour{ 150, 150, 255, 255 };

    t.InitialiseSemantics();
    return t;
}
