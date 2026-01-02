#pragma once
#include <SableUI/utils/utils.h>
#include <optional>
#include <string>
#include <unordered_map>

namespace SableUI
{
	struct Theme
	{
		Colour base;		// Background
		Colour mantle;		// Secondary background
		Colour crust;		// Secondary background

		Colour surface0;	// Elevated
		Colour surface1;	// More elevated
		Colour surface2;	// Even more elevated

		Colour overlay0;	// Darkest overlay
		Colour overlay1;
		Colour overlay2;	// Lightest overlay

		Colour subtext0;	// Very subtle text
		Colour subtext1;	// Subtle text
		Colour text;		// Main

		// Contrasting text colours for theme
		Colour subtext0Contrast;
		Colour subtext1Contrast;
		Colour textContrast;

		Colour rosewater;
		Colour flamingo;
		Colour pink;
		Colour mauve;
		Colour red;
		Colour maroon;
		Colour peach;
		Colour yellow;
		Colour green;
		Colour teal;
		Colour sky;
		Colour sapphire;
		Colour blue;
		Colour lavender;

		Colour primary;
		Colour secondary;
		Colour error;
		Colour warning;
		Colour success;
		Colour info;

		Colour checkColour;

		std::string name;
		std::string description;
		bool isDark;

		void InitialiseSemantics()
		{
			if (primary == Colour{})		primary = blue;
			if (secondary == Colour{})		secondary = rosewater;
			if (error == Colour{})			error = red;
			if (warning == Colour{})		warning = yellow;
			if (success == Colour{})		success = green;
			if (info == Colour{})			info = sky;
			if (checkColour == Colour{})	checkColour = lavender;
		}
	};

	struct ThemeOverride
	{
		std::optional<Colour> base;
		std::optional<Colour> mantle;
		std::optional<Colour> crust;
		std::optional<Colour> surface0;
		std::optional<Colour> surface1;
		std::optional<Colour> surface2;

		std::optional<Colour> overlay0;
		std::optional<Colour> overlay1;
		std::optional<Colour> overlay2;

		std::optional<Colour> subtext0;
		std::optional<Colour> subtext1;
		std::optional<Colour> text;

		std::optional<Colour> subtext0Contrast;
		std::optional<Colour> subtext1Contrast;
		std::optional<Colour> textContrast;

		std::optional<Colour> rosewater;
		std::optional<Colour> flamingo;
		std::optional<Colour> pink;
		std::optional<Colour> mauve;
		std::optional<Colour> red;
		std::optional<Colour> maroon;
		std::optional<Colour> peach;
		std::optional<Colour> yellow;
		std::optional<Colour> green;
		std::optional<Colour> teal;
		std::optional<Colour> sky;
		std::optional<Colour> sapphire;
		std::optional<Colour> blue;
		std::optional<Colour> lavender;

		std::optional<Colour> primary;
		std::optional<Colour> secondary;
		std::optional<Colour> error;
		std::optional<Colour> warning;
		std::optional<Colour> success;
		std::optional<Colour> info;

		Theme Apply(const Theme& base) const;
	};

	class ThemeManager
	{
	public:
		static ThemeManager& GetInstance();

		void RegisterTheme(const std::string& name, const Theme& theme);
		void RegisterThemeVariant(const std::string& name,
			const std::string& baseName,
			const ThemeOverride& override);

		bool SetActiveTheme(const std::string& name);
		const Theme& GetActiveTheme() const;
		const Theme* GetTheme(const std::string& name) const;
		std::vector<std::string> GetThemeNames() const;

	private:
		ThemeManager();
		std::unordered_map<std::string, Theme> m_themes;
		std::string m_activeThemeName = "sableui_dark";
	};

	namespace BuiltInThemes
	{
		Theme SableUIDark();
		Theme SableUILight();
	}

	inline const Theme& GetTheme()
	{
		return ThemeManager::GetInstance().GetActiveTheme();
	}
}
