#include <filesystem>
#include <map>
#include <algorithm>
#include <fstream>
#include <cwctype>
#include <sstream>
#include <vector>
#include <memory>
#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include "SableUI/text.h"

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "SableUI::Text"
#include "SableUI/console.h"
#include <corecrt.h>

constexpr int ATLAS_WIDTH = 512;
constexpr int ATLAS_HEIGHT = 512;
constexpr int ATLAS_PADDING = 2;
constexpr int MIN_ATLAS_DEPTH = 1;
constexpr int MAX_ATLAS_GAP = 0;
constexpr const char* FONT_CACHE_PREFIX = "f-";
constexpr uint32_t ATLAS_CACHE_FILE_VERSION = 1;
constexpr const char* FONT_PACK_CACHE_PREFIX = "fp-";
constexpr uint32_t FONT_PACK_CACHE_FILE_VERSION = 1;
constexpr char32_t MAX_CONTIGUOUS_CHARS = 128;
constexpr int FONT_PACK_DECAY = 10;

static SableUI::vec2 s_dpi = { 96.0f, 96.0f };
void SableUI::SetFontDPI(const vec2& dpi)
{
	s_dpi = dpi;
}

// ============================================================================
// Font Range & Font Pack counters
// ============================================================================
static int s_fontRangeCount = 0;
SableUI::FontRange::FontRange()
{
	s_fontRangeCount++;
}

SableUI::FontRange::FontRange(char32_t start, char32_t end, std::string fontPath)
	: start(start), end(end), fontPath(std::move(fontPath))
{
	s_fontRangeCount++;
}

SableUI::FontRange::FontRange(const FontRange& other)
	: start(other.start), end(other.end), fontPath(other.fontPath)
{
	s_fontRangeCount++;
}

SableUI::FontRange::~FontRange()
{
	s_fontRangeCount--;
}

int SableUI::FontRange::GetNumInstances()
{
	return s_fontRangeCount;
}

static int s_fontPackCount = 0;
SableUI::FontPack::FontPack()
{
	s_fontPackCount++;
}

SableUI::FontPack::FontPack(const FontPack& other)
	: fontPath(other.fontPath), lastConsumed(other.lastConsumed), fontRanges(other.fontRanges)
{
	s_fontPackCount++;
}

SableUI::FontPack::FontPack(FontPack&& other) noexcept
	: fontPath(std::move(other.fontPath)),
	lastConsumed(std::move(other.lastConsumed)),
	fontRanges(std::move(other.fontRanges))
{
	s_fontPackCount++;
}

SableUI::FontPack& SableUI::FontPack::operator=(const FontPack& other)
{
	if (this != &other)
	{
		fontPath = other.fontPath;
		lastConsumed = other.lastConsumed;
		fontRanges = other.fontRanges;
	}
	return *this;
}

SableUI::FontPack& SableUI::FontPack::operator=(FontPack&& other) noexcept
{
	if (this != &other)
	{
		fontPath = std::move(other.fontPath);
		lastConsumed = std::move(other.lastConsumed);
		fontRanges = std::move(other.fontRanges);
	}
	return *this;
}

SableUI::FontPack::~FontPack()
{
	s_fontPackCount--;
}

int SableUI::FontPack::GetNumInstances()
{
	return s_fontPackCount;
}

// ============================================================================
// Data types
// ============================================================================
struct FontPackHint {
	std::string filename;
	char32_t rangeStart;
	char32_t rangeEnd;
	int priority;
};

struct FontRangeHash {
	char data[16] = { 0 };

	std::string ToString() const
	{
		return std::string(reinterpret_cast<const char*>(data));
	}
};

struct Character {
	Character() = default;
	Character(SableUI::u16vec2 pos, SableUI::u16vec2 size, SableUI::vec2 bearing,
		uint16_t advance, uint16_t layer)
		: pos(pos), size(size), bearing(bearing), advance(advance), layer(layer) {}

	SableUI::u16vec2 pos = SableUI::u16vec2(0);
	SableUI::u16vec2 size = SableUI::u16vec2(0);
	SableUI::vec2 bearing = SableUI::vec2(0);
	float advance = 0;
	uint16_t layer = 0;
};

struct Atlas {
	SableUI::FontRange range;
	bool isLoadedFromCache = false;
	int fontSize = 0;
	Atlas() = default;
};

enum class FontType : uint8_t {
	Regular = 0,
	Bold,
	Italic,
	BoldItalic,
	Light,
	LightItalic
};

struct char_t {
	char32_t c = 0;
	int fontSize = 0;
	FontType fontType = FontType::Regular;

	bool operator<(const char_t& other) const {
		if (c != other.c) return c < other.c;
		if (fontSize != other.fontSize) return fontSize < other.fontSize;
		return fontType < other.fontType;
	}
};

// ============================================================================
// Data
// ============================================================================
static std::map<FontType, std::string> directory = {
	{ FontType::Regular, "Regular" },
	{ FontType::Bold, "Bold" },
	{ FontType::Italic, "Italic" },
	{ FontType::BoldItalic, "Bold Italic" },
	{ FontType::Light, "Light" },
	{ FontType::LightItalic, "Light Italic" },
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_REGULAR = {
	// Latin
	{"NotoSans-Regular.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-Regular.ttf", 0x1E00, 0x1EFF, 1},

	// Math symbols
	{"NotoSansMath-Regular.ttf", 0x2200, 0x22FF, 2},
	{"NotoSansMath-Regular.ttf", 0x2A00, 0x2AFF, 2},
	{"NotoSansMath-Regular.ttf", 0x27C0, 0x27EF, 2},
	{"NotoSansMath-Regular.ttf", 0x2980, 0x29FF, 2},

	// Common symbols
	{"NotoSansSymbols-Regular.ttf", 0x2000, 0x206F, 3},
	{"NotoSansSymbols-Regular.ttf", 0x2100, 0x214F, 3},
	{"NotoSansSymbols-Regular.ttf", 0x2190, 0x21FF, 3},
	{"NotoSansSymbols-Regular.ttf", 0x2300, 0x23FF, 3},
	{"NotoSansSymbols-Regular.ttf", 0x2600, 0x26FF, 3},
	{"NotoSansSymbols-Regular.ttf", 0x2700, 0x27BF, 3},

	// Extended symbols
	{"NotoSansSymbols2-Regular.ttf", 0x1F300, 0x1F5FF, 4},
	{"NotoSansSymbols2-Regular.ttf", 0x1F900, 0x1F9FF, 4},

	// CJK
	{"NotoSansJP-Regular.ttf", 0x3040, 0x309F, 5}, // Hiragana
	{"NotoSansJP-Regular.ttf", 0x30A0, 0x30FF, 5}, // Katakana
	{"NotoSansSC-Regular.ttf", 0x4E00, 0x9FFF, 6}, // CJK Unified
	{"NotoSansHK-Regular.ttf", 0x3400, 0x4DBF, 6}, // CJK Extension A
	{"NotoSansKR-Regular.ttf", 0xAC00, 0xD7AF, 7}, // Hangul

	// Devanagari
	{"NotoSansDevanagari-Regular.ttf", 0x0900, 0x097F, 8},

	// Thai
	{"NotoSansThai-Regular.ttf", 0x0E00, 0x0E7F, 9},

	// Emojis
	{"NotoEmoji-Regular.ttf", 0x1F600, 0x1F64F, 99},
	{"NotoEmoji-Regular.ttf", 0x1F300, 0x1F5FF, 99},
	{"NotoEmoji-Regular.ttf", 0x1F680, 0x1F6FF, 99},
	{"NotoEmoji-Regular.ttf", 0x2600, 0x26FF, 99},
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_BOLD = {
	// Latin
	{"NotoSans-Bold.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-Bold.ttf", 0x1E00, 0x1EFF, 1},

	// Common symbols
	{"NotoSansSymbols-Bold.ttf", 0x2000, 0x206F, 2},
	{"NotoSansSymbols-Bold.ttf", 0x2100, 0x214F, 2},
	{"NotoSansSymbols-Bold.ttf", 0x2190, 0x21FF, 2},
	{"NotoSansSymbols-Bold.ttf", 0x2300, 0x23FF, 2},
	{"NotoSansSymbols-Bold.ttf", 0x2600, 0x26FF, 2},
	{"NotoSansSymbols-Bold.ttf", 0x2700, 0x27BF, 2},

	// CJK
	{"NotoSansJP-Bold.ttf", 0x3040, 0x309F, 3}, // Hiragana
	{"NotoSansJP-Bold.ttf", 0x30A0, 0x30FF, 3}, // Katakana
	{"NotoSansSC-Bold.ttf", 0x4E00, 0x9FFF, 4}, // CJK Unified
	{"NotoSansHK-Bold.ttf", 0x3400, 0x4DBF, 4}, // CJK Extension A
	{"NotoSansKR-Bold.ttf", 0xAC00, 0xD7AF, 5}, // Hangul

	// Devanagari
	{"NotoSansDevanagari-Bold.ttf", 0x0900, 0x097F, 6},

	// Thai
	{"NotoSansThai-Bold.ttf", 0x0E00, 0x0E7F, 7},
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_BOLD_ITALIC = {
	// Latin
	{"NotoSans-BoldItalic.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-BoldItalic.ttf", 0x1E00, 0x1EFF, 1},
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_ITALIC = {
	// Latin
	{"NotoSans-Italic.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-Italic.ttf", 0x1E00, 0x1EFF, 1},
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_LIGHT = {
	// Latin
	{"NotoSans-Light.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-Light.ttf", 0x1E00, 0x1EFF, 1},

	// Common symbols
	{"NotoSansSymbols-Light.ttf", 0x2000, 0x206F, 2},
	{"NotoSansSymbols-Light.ttf", 0x2100, 0x214F, 2},
	{"NotoSansSymbols-Light.ttf", 0x2190, 0x21FF, 2},
	{"NotoSansSymbols-Light.ttf", 0x2300, 0x23FF, 2},
	{"NotoSansSymbols-Light.ttf", 0x2600, 0x26FF, 2},
	{"NotoSansSymbols-Light.ttf", 0x2700, 0x27BF, 2},

	// CJK
	{"NotoSansJP-Light.ttf", 0x3040, 0x309F, 3}, // Hiragana
	{"NotoSansJP-Light.ttf", 0x30A0, 0x30FF, 3}, // Katakana
	{"NotoSansSC-Light.ttf", 0x4E00, 0x9FFF, 4}, // CJK Unified
	{"NotoSansHK-Light.ttf", 0x3400, 0x4DBF, 4}, // CJK Extension A
	{"NotoSansKR-Light.ttf", 0xAC00, 0xD7AF, 5}, // Hangul

	// Devanagari
	{"NotoSansDevanagari-Light.ttf", 0x0900, 0x097F, 6},

	// Thai
	{"NotoSansThai-Light.ttf", 0x0E00, 0x0E7F, 7},
};

static const std::vector<FontPackHint> FONT_PACK_HINTS_LIGHT_ITALIC = {
	// Latin
	{"NotoSans-LightItalic.ttf", 0x0000, 0x024F, 1},
	{"NotoSans-LightItalic.ttf", 0x1E00, 0x1EFF, 1},
};

// ============================================================================
// FontManager
// ============================================================================
class FontManager {
public:
	static FontManager& GetInstance();
	bool FreeTypeRunning = false;

	FontManager(FontManager const&) = delete;
	void operator=(FontManager const&) = delete;
	FontManager(FontManager&&) = delete;
	FontManager& operator=(FontManager&&) = delete;

	bool Initialize();
	void Shutdown();
	bool isInitialized = false;
	int GetDrawInfo(SableUI::_Text* text);
	int GetMinWidth(SableUI::_Text* text);
	
	void InitFreeType();
	void ShutdownFreeType();
	FT_Library ft_library = nullptr;

	void ResizeTextureArray(int newDepth);
	GLuint GetAtlasTexture() const { return atlasTextureArray; }

	std::map<char_t, Character> characters;
	bool FindFontRangeForChar(char32_t c, SableUI::FontRange& outRange);

	std::vector<Atlas> atlases;
	std::vector<SableUI::FontPack> cachedFontPacks;
	void LoadFontPack(const std::string& fontFilename);
	void UnloadInactiveFontPacks();
	void MarkFontPackUsed(const std::string& fontPath);
	std::string SuggestFontPackForChar(char32_t c);
	bool LoadFontPackByFilename(const std::string& fontDir, const std::string& filename);

	GLuint atlasTextureArray = 0;
	int atlasDepth = MIN_ATLAS_DEPTH;
	SableUI::u16vec2 atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };

	std::set<std::tuple<SableUI::FontRange, int, FontType>> loadedAtlasKeys;

	FontRangeHash GetAtlasHash(const SableUI::FontRange& range, int fontSize);
	FT_Face GetFontForChar(char32_t c, int fontSize, const std::string& fontPathForAtlas,
		std::map<std::string, FT_Face>& currentLoadedFaces) const;
	void RenderGlyphs(Atlas& atlas);
	void LoadFontRange(Atlas& atlas, const SableUI::FontRange& range);

	void SerialiseAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
		const std::map<char32_t, Character>& charsToSerialise, GLenum pixelType, int initialYOffset);
	bool DeserialiseAtlas(const std::string& filename, Atlas& outAtlas);

	bool SerialiseFontPack(const SableUI::FontPack& pack);
	bool DeserialiseFontPack(const std::string& fontFilename, SableUI::FontPack& outPack);
	std::string GetFontPackCacheFilename(const std::string& fontFilename);

	void UploadAtlasToGPU(int height, int initialY, uint8_t* pixels, GLenum pixelType) const;

private:
	FontManager() : isInitialized(false) {}

	std::chrono::steady_clock::time_point lastDecayCheck;
	FontType currentFontType = FontType::Regular;
	std::string GetCurrentFontDirectory() const;
	const std::vector<FontPackHint>& GetCurrentFontPackHints();
	std::string GetPrimaryFontFilename() const;

	bool SetStyleChar(char32_t c);
};

static FontManager* fontManager = nullptr;

FontManager& FontManager::GetInstance()
{
	static FontManager instance;
	return instance;
}

bool FontManager::Initialize()
{
	if (isInitialized)
	{
		SableUI_Log("FontManager already initialized");
		return true;
	}

	SableUI_Log("FontManager initialized");
	isInitialized = true;

	glGenTextures(1, &atlasTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTextureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, ATLAS_WIDTH, ATLAS_HEIGHT, MIN_ATLAS_DEPTH);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	atlasDepth = MIN_ATLAS_DEPTH;
	fontManager = &GetInstance();

	InitFreeType();

	// Only load the primary font pack on initialization
	LoadFontPackByFilename("fonts/Regular", "NotoSans-Regular.ttf");

	lastDecayCheck = std::chrono::steady_clock::now();

	SableUI_Log("FontManager ready for character requests.");
	return true;
}

void FontManager::Shutdown()
{
	if (!isInitialized) return;

	glDeleteTextures(1, &atlasTextureArray);
	atlasTextureArray = 0;

	atlases.clear();
	characters.clear();
	cachedFontPacks.clear();
	loadedAtlasKeys.clear();

	ShutdownFreeType();
	SableUI_Log("FontManager shut down");
	isInitialized = false;
	fontManager = nullptr;
}

void FontManager::InitFreeType()
{
	SableUI_Log("Initializing FreeType");
	if (FT_Init_FreeType(&ft_library)) SableUI_Runtime_Error("Could not init FreeType library");
	FreeTypeRunning = true;

	FT_Library_SetLcdFilter(ft_library, FT_LCD_FILTER_DEFAULT);
}

void FontManager::ShutdownFreeType()
{
	SableUI_Log("Destroying FreeType");
	if (FreeTypeRunning)
	{
		FT_Done_FreeType(ft_library);
		ft_library = nullptr;
		FreeTypeRunning = false;
	}
}

std::string FontManager::GetCurrentFontDirectory() const
{
	auto it = directory.find(currentFontType);
	if (it != directory.end())
	{
		return "fonts/" + it->second;
	}
	return "fonts/Regular";
}

const std::vector<FontPackHint>& FontManager::GetCurrentFontPackHints()
{
	switch (currentFontType)
	{
	case FontType::Bold:        return FONT_PACK_HINTS_BOLD;
	case FontType::Italic:      return FONT_PACK_HINTS_ITALIC;
	case FontType::BoldItalic:  return FONT_PACK_HINTS_BOLD_ITALIC;
	case FontType::Light:       return FONT_PACK_HINTS_LIGHT;
	case FontType::LightItalic: return FONT_PACK_HINTS_LIGHT_ITALIC;
	case FontType::Regular:
	default:                    return FONT_PACK_HINTS_REGULAR;
	}
}

std::string FontManager::GetPrimaryFontFilename() const
{
	switch (currentFontType)
	{
	case FontType::Bold:        return "NotoSans-Bold.ttf";
	case FontType::Italic:      return "NotoSans-Italic.ttf";
	case FontType::BoldItalic:  return "NotoSans-BoldItalic.ttf";
	case FontType::Light:       return "NotoSans-Light.ttf";
	case FontType::LightItalic: return "NotoSans-LightItalic.ttf";
	case FontType::Regular:
	default:                    return "NotoSans-Regular.ttf";
	}
}

bool FontManager::SetStyleChar(char32_t c)
{
	switch (static_cast<SableUI::StyleTag>(c))
	{
	case SableUI::StyleTag::BoldStart:        currentFontType = FontType::Bold;         return true;
	case SableUI::StyleTag::BoldEnd:          currentFontType = FontType::Regular;      return true;
	case SableUI::StyleTag::ItalicStart:      currentFontType = FontType::Italic;       return true;
	case SableUI::StyleTag::ItalicEnd:        currentFontType = FontType::Regular;      return true;
	case SableUI::StyleTag::BoldItalicStart:  currentFontType = FontType::BoldItalic;   return true;
	case SableUI::StyleTag::BoldItalicEnd:    currentFontType = FontType::Regular;      return true;
	case SableUI::StyleTag::LightStart:       currentFontType = FontType::Light;        return true;
	case SableUI::StyleTag::LightEnd:         currentFontType = FontType::Regular;      return true;
	case SableUI::StyleTag::LightItalicStart: currentFontType = FontType::LightItalic;  return true;
	case SableUI::StyleTag::LightItalicEnd:   currentFontType = FontType::Regular;      return true;
	default: return false;
	}
}

bool FontManager::LoadFontPackByFilename(const std::string& fontDir, const std::string& filename)
{
	if (!FreeTypeRunning)
	{
		SableUI_Runtime_Error("FreeType is not running");
		return false;
	}

	std::filesystem::path fontPath = std::filesystem::path(fontDir) / filename;

	if (!std::filesystem::exists(fontPath))
	{
		SableUI_Error("Font file not found: %s", fontPath.string().c_str());
		return false;
	}

	std::string fontPathStr = fontPath.string();

	for (auto& pack : cachedFontPacks)
	{
		if (pack.fontPath == fontPathStr)
		{
			pack.lastConsumed = std::chrono::steady_clock::now();
			SableUI_Log("Font pack already loaded: %s", filename.c_str());
			return true;
		}
	}

	SableUI::FontPack newPack;

	bool loadedFromCache = DeserialiseFontPack(filename, newPack);

	if (loadedFromCache && newPack.fontPath == fontPathStr)
	{
		cachedFontPacks.push_back(std::move(newPack));
		return true;
	}

	SableUI_Log("Generating font pack from file: %s", filename.c_str());

	FT_Face face;
	if (FT_New_Face(ft_library, fontPathStr.c_str(), 0, &face))
	{
		SableUI_Warn("Could not load font file: %s", fontPathStr.c_str());
		return false;
	}

	FT_Set_Pixel_Sizes(face, 0, 12);
	FT_Set_Char_Size(face, 0, 12 * 64, s_dpi.x, s_dpi.y);

	newPack = SableUI::FontPack();
	newPack.fontPath = fontPathStr;
	newPack.lastConsumed = std::chrono::steady_clock::now();

	FT_ULong charcode = 0;
	FT_UInt glyphIndex = 0;

	charcode = FT_Get_First_Char(face, &glyphIndex);
	if (glyphIndex != 0)
	{
		SableUI::FontRange currentRange = { charcode, charcode, fontPathStr };
		char32_t previousChar = charcode;
		int contiguousCount = 1;

		while (glyphIndex != 0)
		{
			charcode = FT_Get_Next_Char(face, charcode, &glyphIndex);

			if (glyphIndex != 0)
			{
				if (charcode <= previousChar + MAX_ATLAS_GAP + 1 && contiguousCount < MAX_CONTIGUOUS_CHARS)
				{
					currentRange.end = charcode;
					contiguousCount++;
				}
				else
				{
					newPack.fontRanges.push_back(currentRange);
					currentRange = { charcode, charcode, fontPathStr };
					contiguousCount = 1;
				}
				previousChar = charcode;
			}
		}
		newPack.fontRanges.push_back(currentRange);
	}

	FT_Done_Face(face);

	SerialiseFontPack(newPack);

	cachedFontPacks.push_back(std::move(newPack));

	return true;
}

void FontManager::LoadFontPack(const std::string& fontFilename)
{
	LoadFontPackByFilename("fonts/Regular", fontFilename);
}

void FontManager::MarkFontPackUsed(const std::string& fontPath)
{
	for (auto& pack : cachedFontPacks)
	{
		if (pack.fontPath == fontPath)
		{
			pack.lastConsumed = std::chrono::steady_clock::now();
			return;
		}
	}
}

std::string FontManager::SuggestFontPackForChar(char32_t c)
{
	std::string bestMatch;
	int bestPriority = 999;

	const std::vector<FontPackHint>& hints = GetCurrentFontPackHints();

	for (const auto& hint : hints)
	{
		if (c >= hint.rangeStart && c <= hint.rangeEnd)
		{
			if (hint.priority < bestPriority)
			{
				bestPriority = hint.priority;
				bestMatch = hint.filename;
			}
		}
	}

	return bestMatch;
}

bool FontManager::FindFontRangeForChar(char32_t c, SableUI::FontRange& outRange)
{
	if (SableUI::StyleTag::BoldStart <= static_cast<SableUI::StyleTag>(c)
		&& SableUI::StyleTag::LightItalicEnd >= static_cast<SableUI::StyleTag>(c)) return false;

	auto now = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(now - lastDecayCheck).count() >= 1)
	{
		UnloadInactiveFontPacks();
		lastDecayCheck = now;
	}

	std::string currentFontDir = GetCurrentFontDirectory();

	for (auto& pack : cachedFontPacks)
	{
		if (pack.fontPath.find(currentFontDir) == std::string::npos)
			continue;

		for (const auto& range : pack.fontRanges)
		{
			if (c >= range.start && c <= range.end)
			{
				outRange = range;
				pack.lastConsumed = std::chrono::steady_clock::now();
				return true;
			}
		}
	}

	std::string suggestedFont = SuggestFontPackForChar(c);

	if (!suggestedFont.empty())
	{
		SableUI_Log("Character U+%04X not found in %s style, trying suggested font: %s",
			static_cast<unsigned int>(c), directory[currentFontType].c_str(), suggestedFont.c_str());

		if (LoadFontPackByFilename(currentFontDir, suggestedFont))
		{
			for (auto& pack : cachedFontPacks)
			{
				if (pack.fontPath.find(currentFontDir) == std::string::npos)
					continue;

				for (const auto& range : pack.fontRanges)
				{
					if (c >= range.start && c <= range.end)
					{
						outRange = range;
						pack.lastConsumed = std::chrono::steady_clock::now();
						return true;
					}
				}
			}
		}
	}

	SableUI_Log("Character U+%04X not in suggested font for %s, trying style fonts...",
		static_cast<unsigned int>(c), directory[currentFontType].c_str());

	std::vector<std::string> styleFonts;

	const std::vector<FontPackHint>& hints = GetCurrentFontPackHints();
	for (const auto& hint : hints)
	{
		if (std::find(styleFonts.begin(), styleFonts.end(), hint.filename) == styleFonts.end())
		{
			styleFonts.push_back(hint.filename);
		}
	}

	for (const auto& fontFile : styleFonts)
	{
		bool alreadyLoaded = false;
		for (const auto& pack : cachedFontPacks)
		{
			if (pack.fontPath.find(fontFile) != std::string::npos &&
				pack.fontPath.find(currentFontDir) != std::string::npos)
			{
				alreadyLoaded = true;
				break;
			}
		}

		if (alreadyLoaded)
			continue;

		if (LoadFontPackByFilename(currentFontDir, fontFile))
		{
			for (auto& pack : cachedFontPacks)
			{
				if (pack.fontPath.find(fontFile) != std::string::npos &&
					pack.fontPath.find(currentFontDir) != std::string::npos)
				{
					for (const auto& range : pack.fontRanges)
					{
						if (c >= range.start && c <= range.end)
						{
							outRange = range;
							pack.lastConsumed = std::chrono::steady_clock::now();
							SableUI_Log("Found character U+%04X in %s (%s style)",
								static_cast<unsigned int>(c), fontFile.c_str(),
								directory[currentFontType].c_str());
							return true;
						}
					}
				}
			}
		}
	}

	// If still not found and not in Regular, fall back to Regular style
	if (currentFontType != FontType::Regular)
	{
		SableUI_Log("Character U+%04X not found in %s style, falling back to Regular",
			static_cast<unsigned int>(c), directory[currentFontType].c_str());

		FontType previousType = currentFontType;
		currentFontType = FontType::Regular;
		bool found = FindFontRangeForChar(c, outRange);
		currentFontType = previousType;
		return found;
	}

	SableUI_Warn("Character U+%04X not found in any font pack for %s style",
		static_cast<unsigned int>(c), directory[currentFontType].c_str());
	return false;
}

void FontManager::UnloadInactiveFontPacks()
{
	auto now = std::chrono::steady_clock::now();
	auto decayDuration = std::chrono::seconds(FONT_PACK_DECAY);

	auto it = cachedFontPacks.begin();
	while (it != cachedFontPacks.end())
	{
		if (it->fontPath.find("NotoSans-Regular.ttf") != std::string::npos)
		{
			it++;
			continue;
		}

		auto timeSinceLastUse = std::chrono::duration_cast<std::chrono::seconds>(now - it->lastConsumed);

		if (timeSinceLastUse >= decayDuration)
		{
			SableUI_Log("Unloading inactive font pack: %s (unused for %lld seconds)",
				it->fontPath.c_str(), timeSinceLastUse.count());

			std::string fontPathToRemove = it->fontPath;

			auto charIt = characters.begin();
			while (charIt != characters.end())
			{
				bool belongsToFont = false;
				for (const auto& atlas : atlases)
				{
					if (atlas.range.fontPath == fontPathToRemove)
					{
						belongsToFont = true;
						break;
					}
				}

				if (belongsToFont)
				{
					charIt = characters.erase(charIt);
				}
				else
				{
					++charIt;
				}
			}

			atlases.erase(
				std::remove_if(atlases.begin(), atlases.end(),
					[&fontPathToRemove](const Atlas& atlas) {
						return atlas.range.fontPath == fontPathToRemove;
					}),
				atlases.end()
			);

			auto keyIt = loadedAtlasKeys.begin();
			while (keyIt != loadedAtlasKeys.end())
			{
				if (std::get<0>(*keyIt).fontPath == fontPathToRemove)
					keyIt = loadedAtlasKeys.erase(keyIt);
				else
					keyIt++;
			}

			it = cachedFontPacks.erase(it);
		}
		else
			it++;
	}
}

void FontManager::ResizeTextureArray(int newDepth)
{
	if (newDepth <= atlasDepth)
	{
		SableUI_Warn("Trying to resize texture array to a smaller or same depth (old: %i, new: %i)",
			atlasDepth, newDepth);
		return;
	}

	GLuint newTextureArray = 0;
	glGenTextures(1, &newTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, newTextureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, ATLAS_WIDTH, ATLAS_HEIGHT, newDepth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* copy old texture to new, with new depth */
	GLuint oldAtlasTextureArray = atlasTextureArray;
	glCopyImageSubData(oldAtlasTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		newTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		ATLAS_WIDTH, ATLAS_HEIGHT, atlasDepth);

	glDeleteTextures(1, &oldAtlasTextureArray);
	atlasTextureArray = newTextureArray;
	atlasDepth = newDepth;
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

FontRangeHash FontManager::GetAtlasHash(const SableUI::FontRange& range, int fontSize)
{
	// inspired from djb2 hash
	FontRangeHash h;
	unsigned long long hashValue = 5381;

	hashValue = ((hashValue << 5) + hashValue) + fontSize;
	hashValue = ((hashValue << 5) + hashValue) + range.start;
	hashValue = ((hashValue << 5) + hashValue) + range.end;
	hashValue = ((hashValue << 5) + hashValue) + ATLAS_PADDING;

	for (char c : range.fontPath)
	{
		hashValue = ((hashValue << 5) + hashValue) + c;
	}

	std::stringstream ss;
	ss << std::hex << hashValue;
	std::string hexHash = ss.str();

	size_t len = std::min(hexHash.length(), sizeof(h.data) - 1);
	memcpy(h.data, hexHash.c_str(), len);
	h.data[len] = '\0'; // terminate c-style string
	return h;
}

FT_Face FontManager::GetFontForChar(char32_t c, int fontSize, const std::string& fontPathForAtlas,
	std::map<std::string, FT_Face>& currentLoadedFaces) const
{
	if (fontPathForAtlas.empty())
	{
		SableUI_Error("GetFontForChar: Empty font path provided for char U+%04X",
			static_cast<unsigned int>(c));
		return nullptr;
	}

	auto it = currentLoadedFaces.find(fontPathForAtlas);
	if (it != currentLoadedFaces.end())
	{
		if (it->second->size->metrics.x_ppem / 64 != fontSize)
		{
			FT_Set_Pixel_Sizes(it->second, 0, fontSize);
		}
		return it->second;
	}

	FT_Face face;
	if (FT_New_Face(ft_library, fontPathForAtlas.c_str(), 0, &face))
	{
		SableUI_Error("Could not load font: %s for char U+%04X",
			fontPathForAtlas.c_str(), static_cast<unsigned int>(c));
		return nullptr;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
	currentLoadedFaces[fontPathForAtlas] = face;
	SableUI_Log("Loaded font: %s (size %i) for rendering atlas",
		fontPathForAtlas.c_str(), fontSize);
	return face;
}

void FontManager::RenderGlyphs(Atlas& atlas)
{
	SableUI_Log("Rendering glyphs for range: U+%04X - U+%04X (size %i) from %s",
		static_cast<unsigned int>(atlas.range.start),
		static_cast<unsigned int>(atlas.range.end),
		atlas.fontSize,
		atlas.range.fontPath.c_str());

	std::map<std::string, FT_Face> facesForCurrentRender;

	int initialAtlasYForRenderPass = atlasCursor.y;

	/* first pass, get bounds of all glyphs to calculate height of atlas */
	SableUI::uvec2 tempCursor = { ATLAS_PADDING, atlasCursor.y };
	uint16_t currentRowHeight = 0;
	int maxGlobalYReached = atlasCursor.y;

	for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
	{
		FT_Face face = GetFontForChar(c, static_cast<int>(atlas.fontSize),atlas.range.fontPath, facesForCurrentRender);
		if (!face) continue;

		FT_Set_Pixel_Sizes(face, 0, static_cast<int>(atlas.fontSize));
		FT_Set_Char_Size(face, 0, atlas.fontSize * 64, s_dpi.x, s_dpi.y);

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_TARGET_LCD)) continue;

		FT_GlyphSlot glyph = face->glyph;

		FT_Render_Glyph(glyph, FT_RENDER_MODE_LCD);

		SableUI::u16vec2 size = { (uint16_t)glyph->bitmap.width, (uint16_t)glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0)
		{
			tempCursor.x += (glyph->advance.x >> 6) + ATLAS_PADDING;
			maxGlobalYReached = std::max(maxGlobalYReached, (int)tempCursor.y + (int)currentRowHeight);
			continue;
		}

		int currentLayer = tempCursor.y / ATLAS_HEIGHT;
		int potentialEndGlobalY = tempCursor.y + size.y;
		int currentLayerEndGlobalY = (currentLayer + 1) * ATLAS_HEIGHT;

		if (tempCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			tempCursor.x = ATLAS_PADDING;
			tempCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		if (potentialEndGlobalY > currentLayerEndGlobalY)
		{
			tempCursor.x = ATLAS_PADDING;
			tempCursor.y = (currentLayer + 1) * ATLAS_HEIGHT + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);
		tempCursor.x += size.x + ATLAS_PADDING;

		maxGlobalYReached = std::max(maxGlobalYReached, (int)tempCursor.y + (int)currentRowHeight);
	}

	int requiredHeightForPass = maxGlobalYReached - initialAtlasYForRenderPass;
	if (requiredHeightForPass < 0) requiredHeightForPass = 0;

	if (requiredHeightForPass == 0 && (atlas.range.start <= atlas.range.end))
	{
		if (atlas.fontSize > 0)
		{
			requiredHeightForPass = atlas.fontSize + ATLAS_PADDING;
		}
		else
		{
			requiredHeightForPass = ATLAS_PADDING + 1;
		}
	}


	int proposedEndGlobalY = initialAtlasYForRenderPass + requiredHeightForPass;
	int newDepth = static_cast<int>(std::ceil(static_cast<float>(proposedEndGlobalY) / ATLAS_HEIGHT));

	if (newDepth > atlasDepth)
	{
		ResizeTextureArray(newDepth);
	}

	uint8_t* atlasPixels = new uint8_t[static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass * 3];
	std::memset(atlasPixels, 0, static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass * 3);

	currentRowHeight = 0;
	atlasCursor.x = ATLAS_PADDING;
	atlasCursor.y = initialAtlasYForRenderPass;

	std::map<char32_t, Character> charsForSerialization;

	/* second pass, render glyphs and fill the cpu-side buffer */
	for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
	{
		FT_Face face = GetFontForChar(c, atlas.fontSize, atlas.range.fontPath, facesForCurrentRender);
		if (!face) continue;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_TARGET_LCD)) continue;
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LCD);

		FT_GlyphSlot glyph = face->glyph;
		SableUI::u16vec2 size = { (uint16_t)glyph->bitmap.width, (uint16_t)glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0)
		{
			Character emptyChar = Character(
				atlasCursor,
				size,
				SableUI::vec2(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top)),
				static_cast<float>(glyph->advance.x) / 64.0f,
				static_cast<uint16_t>(atlasCursor.y / ATLAS_HEIGHT)
			);

			char_t ct = { c, atlas.fontSize, currentFontType };
			characters[ct] = emptyChar;
			charsForSerialization[c] = emptyChar;

			atlasCursor.x += (glyph->advance.x >> 6) + ATLAS_PADDING;
			continue;
		}

		int currentLayer = atlasCursor.y / ATLAS_HEIGHT;
		int potentialEndGlobalY = atlasCursor.y + size.y;
		int currentLayerEndGlobalY = (currentLayer + 1) * ATLAS_HEIGHT;

		if (atlasCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			atlasCursor.x = ATLAS_PADDING;
			atlasCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		if (potentialEndGlobalY > currentLayerEndGlobalY)
		{
			atlasCursor.x = ATLAS_PADDING;
			atlasCursor.y = (currentLayer + 1) * ATLAS_HEIGHT + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);

		int yOffsetInAtlasPixels = (atlasCursor.y - initialAtlasYForRenderPass);

		if ((yOffsetInAtlasPixels + size.y) > requiredHeightForPass || yOffsetInAtlasPixels < 0)
		{
			SableUI_Error("Atlas overflow during glyph rendering (glyph too tall/misplaced for remaining space)! Char: U+%04X. Corrupt?",
				static_cast<unsigned int>(c));

			char_t ct = { c, atlas.fontSize, currentFontType };
			characters[ct] = Character(
				atlasCursor,
				size,
				SableUI::vec2(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top)),
				static_cast<float>(glyph->advance.x) / 64.0f,
				static_cast<uint16_t>(atlasCursor.y / ATLAS_HEIGHT)
			);
			charsForSerialization[c] = characters[ct];
			atlasCursor.x += (glyph->advance.x >> 6) + ATLAS_PADDING;
			continue;
		}

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				size_t pixelIndexInAtlasPixels = (static_cast<size_t>(yOffsetInAtlasPixels + y) * ATLAS_WIDTH +
					(atlasCursor.x + static_cast<size_t>(x))) * 3;

				if (pixelIndexInAtlasPixels + 2 < static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass * 3)
				{
					if (glyph->bitmap.buffer)
					{
						atlasPixels[pixelIndexInAtlasPixels + 0] = glyph->bitmap.buffer[static_cast<size_t>(y) * 
							glyph->bitmap.pitch + x];
						atlasPixels[pixelIndexInAtlasPixels + 1] = glyph->bitmap.buffer[static_cast<size_t>(y) * 
							glyph->bitmap.pitch + x + 1];
						atlasPixels[pixelIndexInAtlasPixels + 2] = glyph->bitmap.buffer[static_cast<size_t>(y) * 
							glyph->bitmap.pitch + x + 2];
					}
				}
			}
		}

		char_t ct = { c, atlas.fontSize, currentFontType };
		characters[ct] = Character(
			atlasCursor,
			size,
			SableUI::vec2(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top)),
			static_cast<float>(glyph->advance.x) / 64.0f,
			static_cast<uint16_t>(atlasCursor.y / ATLAS_HEIGHT)
		);
		charsForSerialization[c] = characters[ct];

		atlasCursor.x += size.x + ATLAS_PADDING;
	}

	UploadAtlasToGPU(requiredHeightForPass, initialAtlasYForRenderPass, atlasPixels, GL_RGB);

	SerialiseAtlas(atlas, atlasPixels, ATLAS_WIDTH, requiredHeightForPass,
		charsForSerialization, GL_RGB, initialAtlasYForRenderPass);

	delete[] atlasPixels;

	for (auto& pair : facesForCurrentRender)
	{
		FT_Done_Face(pair.second);
	}

	atlasCursor.y = initialAtlasYForRenderPass + requiredHeightForPass + ATLAS_PADDING;
}

std::string FontManager::GetFontPackCacheFilename(const std::string& fontFilename)
{
	unsigned long long hashValue = 5381;
	for (char c : fontFilename)
	{
		hashValue = ((hashValue << 5) + hashValue) + c;
	}

	std::stringstream ss;
	ss << std::hex << hashValue;

	return "cache/" + std::string(FONT_PACK_CACHE_PREFIX) + ss.str() + ".sbfontpack";
}

bool FontManager::SerialiseFontPack(const SableUI::FontPack& pack)
{
	std::filesystem::path fontPath(pack.fontPath);
	std::string fontFilename = fontPath.filename().string();

	std::string cacheFilename = GetFontPackCacheFilename(fontFilename);

	try
	{
		std::filesystem::create_directories("cache/");
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		SableUI_Error("Failed to create cache directory: %s", e.what());
		return false;
	}

	std::ofstream file(cacheFilename, std::ios::binary);

	if (!file.is_open())
	{
		SableUI_Error("Could not open file for writing font pack cache: %s", cacheFilename.c_str());
		return false;
	}

	try
	{
		// header
		file.write(reinterpret_cast<const char*>(&FONT_PACK_CACHE_FILE_VERSION), sizeof(uint32_t));

		// font path
		size_t fontPathLen = pack.fontPath.length();
		file.write(reinterpret_cast<const char*>(&fontPathLen), sizeof(size_t));
		file.write(pack.fontPath.c_str(), fontPathLen);

		size_t numRanges = pack.fontRanges.size();
		file.write(reinterpret_cast<const char*>(&numRanges), sizeof(size_t));

		// data
		for (const auto& range : pack.fontRanges)
		{
			file.write(reinterpret_cast<const char*>(&range.start), sizeof(char32_t));
			file.write(reinterpret_cast<const char*>(&range.end), sizeof(char32_t));

			size_t rangePathLen = range.fontPath.length();
			file.write(reinterpret_cast<const char*>(&rangePathLen), sizeof(size_t));
			file.write(range.fontPath.c_str(), rangePathLen);
		}

		file.close();
		SableUI_Log("Saved font pack cache: %s (%zu ranges)", fontFilename.c_str(), numRanges);
		return true;
	}
	catch (const std::exception& e)
	{
		SableUI_Error("Error serializing font pack %s: %s", fontFilename.c_str(), e.what());
		file.close();
		std::filesystem::remove(cacheFilename);
		return false;
	}
}

bool FontManager::DeserialiseFontPack(const std::string& fontFilename, SableUI::FontPack& outPack)
{
	std::string cacheFilename = GetFontPackCacheFilename(fontFilename);

	if (!std::filesystem::exists(cacheFilename))
	{
		return false;
	}

	std::ifstream file(cacheFilename, std::ios::binary);

	if (!file.is_open())
	{
		SableUI_Error("Could not open font pack cache file: %s", cacheFilename.c_str());
		return false;
	}

	try
	{
		uint32_t fileVersion{};
		file.read(reinterpret_cast<char*>(&fileVersion), sizeof(uint32_t));

		if (fileVersion != FONT_PACK_CACHE_FILE_VERSION)
		{
			SableUI_Warn("Font pack cache version mismatch for %s. Expected %u, got %u. Regenerating.",
				fontFilename.c_str(), FONT_PACK_CACHE_FILE_VERSION, fileVersion);
			file.close();
			std::filesystem::remove(cacheFilename);
			return false;
		}

		size_t fontPathLen{};
		file.read(reinterpret_cast<char*>(&fontPathLen), sizeof(size_t));
		outPack.fontPath.resize(fontPathLen);
		file.read(outPack.fontPath.data(), fontPathLen);

		if (!std::filesystem::exists(outPack.fontPath))
		{
			SableUI_Warn("Cached font path no longer exists: %s. Removing cache.", outPack.fontPath.c_str());
			file.close();
			std::filesystem::remove(cacheFilename);
			return false;
		}

		size_t numRanges{};
		file.read(reinterpret_cast<char*>(&numRanges), sizeof(size_t));

		outPack.fontRanges.clear();
		outPack.fontRanges.reserve(numRanges);

		for (size_t i = 0; i < numRanges; i++)
		{
			SableUI::FontRange range;

			file.read(reinterpret_cast<char*>(&range.start), sizeof(char32_t));
			file.read(reinterpret_cast<char*>(&range.end), sizeof(char32_t));

			size_t rangePathLen{};
			file.read(reinterpret_cast<char*>(&rangePathLen), sizeof(size_t));
			range.fontPath.resize(rangePathLen);
			file.read(range.fontPath.data(), rangePathLen);

			outPack.fontRanges.push_back(range);
		}

		outPack.lastConsumed = std::chrono::steady_clock::now();

		file.close();

		if (!file)
		{
			SableUI_Error("Error reading font pack cache: %s", cacheFilename.c_str());
			std::filesystem::remove(cacheFilename);
			return false;
		}

		SableUI_Log("Loaded font pack from cache: %s (%zu ranges)", fontFilename.c_str(), numRanges);
		return true;
	}
	catch (const std::exception& e)
	{
		SableUI_Error("Error deserializing font pack %s: %s", fontFilename.c_str(), e.what());
		file.close();
		std::filesystem::remove(cacheFilename);
		return false;
	}
}

void FontManager::UploadAtlasToGPU(int height, int initialY, uint8_t* pixels, GLenum pixelType) const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTextureArray);

	int currentUploadY = 0;
	while (currentUploadY < height)
	{
		int globalYForChunk = initialY + currentUploadY;

		int targetLayer = globalYForChunk / ATLAS_HEIGHT;
		int yOffsetInTargetLayer = globalYForChunk % ATLAS_HEIGHT;

		int heightToUpload = std::min(ATLAS_HEIGHT - yOffsetInTargetLayer, height - currentUploadY);

		if (heightToUpload <= 0) break;

		const uint8_t* chunkPixels = pixels + (static_cast<size_t>(currentUploadY) *
											   ATLAS_WIDTH * (pixelType == GL_RGB ? 3 : 1));

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, yOffsetInTargetLayer, targetLayer,
			ATLAS_WIDTH, heightToUpload, 1, pixelType, GL_UNSIGNED_BYTE, chunkPixels);

		currentUploadY += heightToUpload;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void FontManager::SerialiseAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
	const std::map<char32_t, Character>& charsToSerialise, GLenum pixelType, int initialYOffset)
{
	FontRangeHash atlasHash = GetAtlasHash(atlas.range, atlas.fontSize);
	std::string directory = "cache/";
	std::string filename = directory + FONT_CACHE_PREFIX + atlasHash.ToString() + ".sbatlas";

	try
	{
		std::filesystem::create_directories(directory);
	}
	catch (const std::filesystem::filesystem_error& e) {
		SableUI_Error("Failed to create cache directory %s: %s", directory.c_str(), e.what());
		return;
	}

	std::ofstream file(filename, std::ios::binary);

	if (!file.is_open())
	{
		SableUI_Error("Could not open file for writing cache: %s", filename.c_str());
		return;
	}

	/* header */
	file.write(reinterpret_cast<const char*>(&ATLAS_CACHE_FILE_VERSION), sizeof(uint32_t));
	file.write(reinterpret_cast<const char*>(&atlas.fontSize), sizeof(float));
	file.write(reinterpret_cast<const char*>(&atlas.range.start), sizeof(char32_t));
	file.write(reinterpret_cast<const char*>(&atlas.range.end), sizeof(char32_t));
	size_t path_len = atlas.range.fontPath.length();
	file.write(reinterpret_cast<const char*>(&path_len), sizeof(size_t));
	file.write(atlas.range.fontPath.c_str(), path_len);

	file.write(reinterpret_cast<const char*>(&width), sizeof(int));
	file.write(reinterpret_cast<const char*>(&height), sizeof(int));
	file.write(reinterpret_cast<const char*>(&pixelType), sizeof(GLenum));
	file.write(reinterpret_cast<const char*>(&initialYOffset), sizeof(int));


	/* main content */
	size_t bytesPerPixel = (pixelType == GL_RGB ? 3 : 1);
	size_t pxArraySize = static_cast<size_t>(width) * height * bytesPerPixel;
	file.write(reinterpret_cast<const char*>(pixels), pxArraySize * sizeof(uint8_t));

	size_t num_chars = charsToSerialise.size();
	file.write(reinterpret_cast<const char*>(&num_chars), sizeof(size_t));
	for (const auto& pair : charsToSerialise)
	{
		char32_t char_code = pair.first;
		Character character = pair.second;

		character.pos.y = character.pos.y - initialYOffset;
		character.layer = character.pos.y / ATLAS_HEIGHT;


		file.write(reinterpret_cast<const char*>(&char_code), sizeof(char32_t));
		file.write(reinterpret_cast<const char*>(&character.pos.x), sizeof(uint16_t));
		file.write(reinterpret_cast<const char*>(&character.pos.y), sizeof(uint16_t));
		file.write(reinterpret_cast<const char*>(&character.size.x), sizeof(uint16_t));
		file.write(reinterpret_cast<const char*>(&character.size.y), sizeof(uint16_t));
		file.write(reinterpret_cast<const char*>(&character.bearing.x), sizeof(float));
		file.write(reinterpret_cast<const char*>(&character.bearing.y), sizeof(float));
		file.write(reinterpret_cast<const char*>(&character.advance), sizeof(float));
		file.write(reinterpret_cast<const char*>(&character.layer), sizeof(uint16_t));
	}

	file.close();
	SableUI_Log("Saved atlas to %s", filename.c_str());
}

bool FontManager::DeserialiseAtlas(const std::string& filename, Atlas& outAtlas)
{
	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open())
	{
		SableUI_Error("Could not open file for reading cache: %s", filename.c_str());
		return false;
	}

	try
	{
		/* header */
		uint32_t fileVersion{};
		file.read(reinterpret_cast<char*>(&fileVersion), sizeof(uint32_t));
		if (fileVersion != ATLAS_CACHE_FILE_VERSION)
		{
			SableUI_Warn("Atlas cache file version mismatch! File: %s. Expected %u, got %u. Regenerating.",
				filename.c_str(), ATLAS_CACHE_FILE_VERSION, fileVersion);
			file.close();
			return false;
		}

		file.read(reinterpret_cast<char*>(&outAtlas.fontSize), sizeof(int));
		file.read(reinterpret_cast<char*>(&outAtlas.range.start), sizeof(char32_t));
		file.read(reinterpret_cast<char*>(&outAtlas.range.end), sizeof(char32_t));
		size_t path_len{};
		file.read(reinterpret_cast<char*>(&path_len), sizeof(size_t));
		outAtlas.range.fontPath.resize(path_len);
		file.read(outAtlas.range.fontPath.data(), path_len);

		int cachedWidth{};
		int cachedHeight{};
		file.read(reinterpret_cast<char*>(&cachedWidth), sizeof(int));
		file.read(reinterpret_cast<char*>(&cachedHeight), sizeof(int));
		GLenum cachedPixelType{};
		file.read(reinterpret_cast<char*>(&cachedPixelType), sizeof(GLenum));
		int originalInitialYOffset{};
		file.read(reinterpret_cast<char*>(&originalInitialYOffset), sizeof(int));


		if (cachedWidth != ATLAS_WIDTH)
		{
			SableUI_Warn("Cached atlas width mismatch for %s. Expected %i, got %i. Regenerating.",
				filename.c_str(), ATLAS_WIDTH, cachedWidth);
			file.close();
			return false;
		}
		if (cachedPixelType != GL_RGB)
		{
			SableUI_Warn("Cached atlas pixel format mismatch for %s. Expected GL_RGB, got %u. Regenerating.",
				filename.c_str(), cachedPixelType);
			file.close();
			return false;
		}

		/* main content */
		size_t bytesPerPixel = (cachedPixelType == GL_RGB ? 3 : 1);
		size_t pxArraySize = static_cast<size_t>(cachedWidth) * cachedHeight * bytesPerPixel;
		std::unique_ptr<uint8_t[]> pixels_buffer = std::make_unique<uint8_t[]>(pxArraySize);
		file.read(reinterpret_cast<char*>(pixels_buffer.get()), pxArraySize * sizeof(uint8_t));

		if (!file)
		{
			SableUI_Error("Error reading pixel data from cache file: %s", filename.c_str());
			file.close();
			return false;
		}

		int initialAtlasYForDeserialisedPass = atlasCursor.y;

		int proposedEndGlobalY = initialAtlasYForDeserialisedPass + cachedHeight;
		int requiredDepthForDeserialization = static_cast<int>(std::ceil(static_cast<float>(proposedEndGlobalY) / ATLAS_HEIGHT));

		if (requiredDepthForDeserialization > atlasDepth)
			ResizeTextureArray(requiredDepthForDeserialization);

		UploadAtlasToGPU(cachedHeight, initialAtlasYForDeserialisedPass, pixels_buffer.get(), cachedPixelType);

		atlasCursor.y += cachedHeight + ATLAS_PADDING;

		size_t num_chars{};
		file.read(reinterpret_cast<char*>(&num_chars), sizeof(size_t));

		for (size_t i = 0; i < num_chars; i++)
		{
			char32_t charCode{};
			Character character{};
			file.read(reinterpret_cast<char*>(&charCode), sizeof(char32_t));
			file.read(reinterpret_cast<char*>(&character.pos.x), sizeof(uint16_t));
			file.read(reinterpret_cast<char*>(&character.pos.y), sizeof(uint16_t));
			file.read(reinterpret_cast<char*>(&character.size.x), sizeof(uint16_t));
			file.read(reinterpret_cast<char*>(&character.size.y), sizeof(uint16_t));
			file.read(reinterpret_cast<char*>(&character.bearing.x), sizeof(float));
			file.read(reinterpret_cast<char*>(&character.bearing.y), sizeof(float));
			file.read(reinterpret_cast<char*>(&character.advance), sizeof(float));
			file.read(reinterpret_cast<char*>(&character.layer), sizeof(uint16_t));

			character.pos.y = character.pos.y + initialAtlasYForDeserialisedPass;
			character.layer = static_cast<uint16_t>(character.pos.y / ATLAS_HEIGHT);

			char_t key = { charCode, outAtlas.fontSize, currentFontType };
			characters[key] = character;
		}

		file.close();
		outAtlas.isLoadedFromCache = true;
		return true;
	}
	catch (const std::exception& e)
	{
		SableUI_Error("Error during atlas deserialization from %s: %s",
			filename.c_str(), e.what());

		file.close();
		std::filesystem::remove(filename);
		return false;
	}
}

void FontManager::LoadFontRange(Atlas& atlas, const SableUI::FontRange& range)
{
	atlas.range = range;

	std::tuple<SableUI::FontRange, int, FontType> currentAtlasKey = { range, atlas.fontSize, currentFontType };
	if (loadedAtlasKeys.count(currentAtlasKey))
	{
		SableUI_Log("Atlas for range U+%04X - U+%04X (size %i) already loaded. Skipping.",
			static_cast<unsigned int>(range.start), static_cast<unsigned int>(range.end), atlas.fontSize);

		return;
	}

	bool loadedFromCache = false;
	FontRangeHash hash = GetAtlasHash(range, atlas.fontSize);
	std::string directory = "cache/";
	std::string filename = directory + FONT_CACHE_PREFIX + hash.ToString() + ".sbatlas";

	if (std::filesystem::exists(filename))
	{
		if (DeserialiseAtlas(filename, atlas))
		{
			loadedFromCache = true;
		}
		else
		{
			SableUI_Warn("Failed to deserialise atlas from %s. Regenerating...", filename.c_str());
			loadedFromCache = false;
		}
	}

	if (!loadedFromCache)
	{
		RenderGlyphs(atlas);
	}

	atlases.emplace_back(atlas);
	loadedAtlasKeys.insert(currentAtlasKey);
}

struct Vertex {
	SableUI::vec2 pos;
	SableUI::vec3 uv; // uv.x and uv.y for texture coordinates, uv.z for layer
	SableUI::Colour colour;	
};

struct TextToken {
	std::u32string content;
	float width = 0.0f;
	bool isSpace = false;
	bool isNewline = false;
	std::vector<Character> charDataList;
};

int FontManager::GetDrawInfo(SableUI::_Text* text)
{
	SableUI::TextJustification currentJustification = text->m_justify;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int height = 0;

	std::vector<TextToken> tokens;
	TextToken currentToken;

	currentFontType = FontType::Regular;

	/* tokenisation with style tracking */
	for (char32_t c : text->m_content)
	{
		if (SetStyleChar(c))
		{
			if (!currentToken.content.empty())
			{
				tokens.push_back(currentToken);
				currentToken = TextToken{};
			}
			continue;
		}

		if (c == U'\n')
		{
			if (!currentToken.content.empty())
				tokens.push_back(currentToken);

			TextToken newlineToken;
			newlineToken.isNewline = true;
			tokens.push_back(newlineToken);
			currentToken = TextToken{};
			continue;
		}

		Character charData;
		char_t charKey = { c, text->m_fontSize, currentFontType };

		std::string compositeKey = std::to_string(c) + "_" +
			std::to_string(text->m_fontSize) + "_" +
			std::to_string(static_cast<int>(currentFontType));

		auto it = characters.find(charKey);

		bool needsReload = false;
		if (it != characters.end())
		{
			std::string currentFontDir = GetCurrentFontDirectory();
			bool foundInStyle = false;

			for (const auto& atlas : atlases)
			{
				if (atlas.fontSize == text->m_fontSize &&
					atlas.range.fontPath.find(currentFontDir) != std::string::npos)
				{
					if (c >= atlas.range.start && c <= atlas.range.end)
					{
						foundInStyle = true;
						break;
					}
				}
			}

			if (!foundInStyle)
				needsReload = true;
			else
				charData = it->second;
		}
		else
		{
			needsReload = true;
		}

		if (needsReload)
		{
			SableUI::FontRange targetRange;
			if (FindFontRangeForChar(c, targetRange))
			{
				Atlas newAtlas{};
				newAtlas.fontSize = text->m_fontSize;
				LoadFontRange(newAtlas, targetRange);

				it = characters.find(charKey);
				if (it != characters.end())
					charData = it->second;
				else
				{
					SableUI_Warn("Could not find character U+%04X with font size %d in %s style even after loading. Using empty glyph.",
						c, text->m_fontSize, directory[currentFontType].c_str());
					charData = Character{};
				}
			}
			else
			{
				SableUI_Warn("Could not find font range for character U+%04X in %s style. Using empty glyph.",
					c, directory[currentFontType].c_str());
				charData = Character{};
			}
		}

		if (std::iswspace(static_cast<wint_t>(c)))
		{
			if (!currentToken.content.empty() && !currentToken.isSpace)
			{
				tokens.push_back(currentToken);
				currentToken = TextToken{};
			}
			if (currentToken.content.empty() || !currentToken.isSpace)
				currentToken.isSpace = true;

			currentToken.content.push_back(c);
			currentToken.width += charData.advance;
			currentToken.charDataList.push_back(charData);
		}
		else
		{
			if (!currentToken.content.empty() && currentToken.isSpace)
			{
				tokens.push_back(currentToken);
				currentToken = TextToken{};
			}
			currentToken.isSpace = false;
			currentToken.content.push_back(c);
			currentToken.width += charData.advance;
			currentToken.charDataList.push_back(charData);
		}
	}

	if (!currentToken.content.empty())
		tokens.push_back(currentToken);

	SableUI::vec2 cursor{};
	currentFontType = FontType::Regular;

	/* pass 1 */
	std::vector<std::vector<TextToken>> lines;
	std::vector<TextToken> currentLine;
	float currentLineWidth = 0.0f;
	int totalLines = 1;

	for (const TextToken& token : tokens)
	{
		if (token.isNewline)
		{
			lines.push_back(currentLine);
			currentLine.clear();
			currentLineWidth = 0;
			totalLines++;
			continue;
		}

		if (!token.isSpace && text->m_maxWidth > 0 &&
			currentLineWidth > 0 &&
			(currentLineWidth + token.width > text->m_maxWidth))
		{
			lines.push_back(currentLine);
			currentLine.clear();
			currentLineWidth = 0;
			totalLines++;
		}

		currentLine.push_back(token);
		currentLineWidth += token.width;
	}
	if (!currentLine.empty())
		lines.push_back(currentLine);

	height = totalLines * text->m_lineSpacingPx;
	cursor.y -= height;

	/* pass 2 */
	unsigned int currentGlyphOffset = 0;

	for (const auto& line : lines)
	{
		float lineWidth = 0.0f;
		for (const auto& token : line)
			lineWidth += token.width;

		float xOffset = 0.0f;
		if (text->m_maxWidth > 0)
		{
			switch (currentJustification)
			{
			case SableUI::TextJustification::Center:
				xOffset = (text->m_maxWidth - lineWidth) / 2.0f;
				break;
			case SableUI::TextJustification::Right:
				xOffset = (text->m_maxWidth - lineWidth);
				break;
			case SableUI::TextJustification::Left:
			default:
				xOffset = 0.0f;
				break;
			}
		}

		cursor.x = xOffset;

		for (const auto& token : line)
		{
			for (size_t i = 0; i < token.content.length(); i++)
			{
				const Character& charData = token.charDataList[i];

				float x = std::round(cursor.x + charData.bearing.x);
				float y = std::round(cursor.y - charData.bearing.y + static_cast<float>(text->m_fontSize));

				float w = static_cast<float>(charData.size.x) / 3.0f;
				float h = static_cast<float>(charData.size.y);

				float uBottomLeft = static_cast<float>(charData.pos.x) / ATLAS_WIDTH;
				float vBottomLeft = static_cast<float>(charData.pos.y % ATLAS_HEIGHT) / ATLAS_HEIGHT;

				float uTopRight = uBottomLeft + (static_cast<float>(charData.size.x) / ATLAS_WIDTH);
				float vTopRight = vBottomLeft + (static_cast<float>(charData.size.y) / ATLAS_HEIGHT);

				float layerIndex = static_cast<float>(charData.layer);

				vertices.push_back(Vertex{ {x, y},           {uBottomLeft, vBottomLeft, layerIndex}, text->m_colour });
				vertices.push_back(Vertex{ {x + w, y},       {uTopRight,   vBottomLeft, layerIndex}, text->m_colour });
				vertices.push_back(Vertex{ {x + w, y + h},   {uTopRight,   vTopRight,   layerIndex}, text->m_colour });
				vertices.push_back(Vertex{ {x, y + h},       {uBottomLeft, vTopRight,   layerIndex}, text->m_colour });

				unsigned int offset = currentGlyphOffset * 4;
				indices.push_back(offset);
				indices.push_back(offset + 1);
				indices.push_back(offset + 2);
				indices.push_back(offset);
				indices.push_back(offset + 2);
				indices.push_back(offset + 3);

				cursor.x += charData.advance;
				currentGlyphOffset++;
			}
		}
		cursor.y += text->m_lineSpacingPx;
	}

	glBindVertexArray(text->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text->m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	text->indiciesSize = indices.size();

	glBindVertexArray(0);
	return height;
}

int FontManager::GetMinWidth(SableUI::_Text* text)
{
	int maxWordWidth = 0;
	int currentWordWidth = 0;

	for (char32_t c : text->m_content)
	{
		if (c == U' ' || c == U'\n' || c == U'\t')
		{
			maxWordWidth = std::max(maxWordWidth, currentWordWidth);
			currentWordWidth = 0;
			continue;
		}

		SetStyleChar(c);

		char_t charKey = { c, text->m_fontSize, currentFontType };
		auto it = characters.find(charKey);

		if (it != characters.end())
		{
			currentWordWidth += it->second.advance;
		}
		else
		{
			SableUI::FontRange targetRange;
			if (FindFontRangeForChar(c, targetRange))
			{
				Atlas newAtlas{};
				newAtlas.fontSize = text->m_fontSize;
				fontManager->LoadFontRange(newAtlas, targetRange);
				auto newIt = fontManager->characters.find(charKey);
				if (newIt != fontManager->characters.end())
				{
					currentWordWidth += newIt->second.advance;
				}
			}
		}
	}

	currentFontType = FontType::Regular;

	return std::max(maxWordWidth, currentWordWidth);
}

GLuint SableUI::GetAtlasTexture()
{
	if (!FontManager::GetInstance().isInitialized)
		FontManager::GetInstance().Initialize();
	
	return FontManager::GetInstance().GetAtlasTexture();
}

// ============================================================================
// Text Backend
// ============================================================================
int SableUI::_Text::SetContent(const SableString& str, int maxWidth, int fontSize, float lineSpacingFac, TextJustification justify)
{
	// Ensure FontManager is initialized
	if (fontManager == nullptr || !fontManager->isInitialized)
		FontManager::GetInstance().Initialize();

	m_content = str;
	m_fontSize = fontSize;
	m_lineSpacingPx = fontSize * lineSpacingFac;
	m_maxWidth = maxWidth;
	m_justify = justify;

	int height = fontManager->GetDrawInfo(this);
	return height;
}

int SableUI::_Text::UpdateMaxWidth(int maxWidth)
{
	// Ensure FontManager is initialized
	if (fontManager == nullptr || !fontManager->isInitialized)
		FontManager::GetInstance().Initialize();

	m_maxWidth = maxWidth;

	int height = fontManager->GetDrawInfo(this);
	return height;
}

int SableUI::_Text::GetMinWidth()
{
	if (fontManager == nullptr || !fontManager->isInitialized)
		FontManager::GetInstance().Initialize();

	fontManager = &FontManager::GetInstance();

	return fontManager->GetMinWidth(this);
}

int SableUI::_Text::GetUnwrappedHeight()
{
	int lines = 1;
	for (char32_t c : m_content)
	{
		if (c == U'\n')
		{
			lines++;
		}
	}

	return lines * m_lineSpacingPx;
}

void SableUI::InitFontManager()
{
	if (fontManager == nullptr)
		FontManager::GetInstance().Initialize();
}

void SableUI::DestroyFontManager()
{
	FontManager::GetInstance().Shutdown();
}

static int s_textOGL = 0;

SableUI::_Text::_Text()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	// Position attribute (layout 0, 2 floats) - 8 bytes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// UV + Layer attribute (layout 1, 3 floats) - 12 bytes
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(vec2)));
	glEnableVertexAttribArray(1);

	// Colour (layout 2, uint32_t) - 4 bytes
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(sizeof(vec2) + sizeof(vec3)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	s_textOGL++;
}

SableUI::_Text::~_Text()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	m_VAO = 0; m_VBO = 0; m_EBO = 0;
	s_textOGL--;
}

int SableUI::_Text::GetNumInstances()
{
	return s_textOGL;
}
