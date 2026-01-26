#include <SableUI/styles/theme.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/utils.h>
#include <string>
#include <vector>

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
	RegisterTheme("sableui_light", BuiltInThemes::SableUILight());
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

	t.base = Colour{ 25,  25,  25, 255 };
	t.mantle = Colour{ 23,  23,  23, 255 };
	t.crust = Colour{ 20,  20,  20, 255 };

	t.surface0 = Colour{ 35,  35,  35, 255 };
	t.surface1 = Colour{ 40,  40,  40, 255 };
	t.surface2 = Colour{ 48,  48,  48, 255 };

	t.overlay0 = Colour{ 55,  55,  55, 255 };
	t.overlay1 = Colour{ 75,  75,  75, 255 };
	t.overlay2 = Colour{ 100, 100, 100, 255 };

	t.subtext0 = Colour{ 140, 140, 140, 255 };
	t.subtext1 = Colour{ 180, 180, 180, 255 };
	t.text = Colour{ 235, 235, 235, 255 };

	t.subtext0Contrast = Colour{ 120, 120, 120, 255 };
	t.subtext1Contrast = Colour{ 80,  80,  80, 255 };
	t.textContrast = Colour{ 20,  20,  20, 255 };

	t.rosewater = Colour{ 220, 138, 120, 255 };
	t.flamingo = Colour{ 221, 120, 120, 255 };
	t.pink = Colour{ 234, 118, 203, 255 };
	t.mauve = Colour{ 136, 57, 239, 255 };
	t.red = Colour{ 210, 15, 57, 255 };
	t.maroon = Colour{ 230, 69, 83, 255 };
	t.peach = Colour{ 254, 100, 11, 255 };
	t.yellow = Colour{ 223, 142, 29, 255 };
	t.green = Colour{ 64, 160, 43, 255 };
	t.teal = Colour{ 23, 146, 153, 255 };
	t.sky = Colour{ 4, 165, 229, 255 };
	t.sapphire = Colour{ 32, 159, 181, 255 };
	t.blue = Colour{ 30, 102, 245, 255 };
	t.lavender = Colour{ 114, 135, 253, 255 };

	t.primary = Colour{ 18, 80, 118, 255 };
	t.secondary = t.overlay1;

	t.success = Colour{ 50, 255, 50, 255 };
	t.warning = Colour{ 255, 200, 50, 255 };
	t.error = Colour{ 255, 50, 50, 255 };
	t.info = t.subtext0;

	t.InitialiseSemantics();
	return t;
}

Theme SableUI::BuiltInThemes::SableUILight()
{
	Theme t;
	t.name = "SableUI Light";
	t.description = "Original SableUI dark theme";
	t.isDark = false;

	t.base = Colour{ 250, 250, 250, 255 };
	t.mantle = Colour{ 245, 245, 245, 255 };
	t.crust = Colour{ 240, 240, 240, 255 };

	t.surface0 = Colour{ 255, 255, 255, 255 };
	t.surface1 = Colour{ 240, 240, 240, 255 };
	t.surface2 = Colour{ 225, 225, 225, 255 };

	t.overlay0 = Colour{ 210, 210, 210, 255 };
	t.overlay1 = Colour{ 190, 190, 190, 255 };
	t.overlay2 = Colour{ 165, 165, 165, 255 };

	t.subtext0 = Colour{ 120, 120, 120, 255 };
	t.subtext1 = Colour{ 80,  80,  80, 255 };
	t.text = Colour{ 20,  20,  20, 255 };

	t.subtext0Contrast = Colour{ 140, 140, 140, 255 };
	t.subtext1Contrast = Colour{ 180, 180, 180, 255 };
	t.textContrast = Colour{ 235, 235, 235, 255 };

	t.rosewater = Colour{ 220, 138, 120, 255 };
	t.flamingo = Colour{ 221, 120, 120, 255 };
	t.pink = Colour{ 234, 118, 203, 255 };
	t.mauve = Colour{ 136, 57, 239, 255 };
	t.red = Colour{ 210, 15, 57, 255 };
	t.maroon = Colour{ 230, 69, 83, 255 };
	t.peach = Colour{ 254, 100, 11, 255 };
	t.yellow = Colour{ 223, 142, 29, 255 };
	t.green = Colour{ 64, 160, 43, 255 };
	t.teal = Colour{ 23, 146, 153, 255 };
	t.sky = Colour{ 4, 165, 229, 255 };
	t.sapphire = Colour{ 32, 159, 181, 255 };
	t.blue = Colour{ 30, 102, 245, 255 };
	t.lavender = Colour{ 114, 135, 253, 255 };

	t.primary = t.overlay0;
	t.secondary = t.overlay1;

	t.success = Colour{ 40, 120, 40, 255 };
	t.warning = Colour{ 120, 120, 26, 255 };
	t.error = Colour{ 117, 54, 46, 255 };
	t.info = t.subtext0;

	t.InitialiseSemantics();
	return t;
}