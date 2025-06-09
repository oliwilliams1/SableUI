#include <GL/glew.h>
#include "SableUI/text.h"
#include "SableUI/utils.h"
#include "SableUI/console.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <filesystem>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <array>

struct FontRange {
    char32_t start = 0;
    char32_t end = 0;
    std::string fontPath = "";
};

struct FontRangeHash {
    char data[16] = { 0 };

    std::string ToString() const
    {
        return std::string(reinterpret_cast<const char*>(data));
    }
};

struct Character {
    GLuint textureID;
    SableUI::ivec2 size;
    SableUI::ivec2 bearing;
    unsigned int advance;
    SableUI::vec4 texCoords;
};

struct Atlas {
    FontRange range;
    std::map<char32_t, Character> characters;
    GLuint textureID = 0;
};

static std::vector<FontRange> fontRanges = {
    {0x0000,    0x2FFF,    "fonts/NotoSans-Regular.ttf"},   /* Basic Latin->Ideographic Description Characters
                                                               (just before CJK) */
    {0x2600,    0xD7A3,    "fonts/NotoSansKR-Regular.ttf"}, // Korean (INNACURATE - FIX)
    {0x3040,    0x30FF,    "fonts/NotoSansJP-Regular.ttf"}, // Hiragana and Katakana
    {0x4E00,    0x9FFF,    "fonts/NotoSansSC-Regular.ttf"}, // CJK Unified Ideographs
    {0x1F300,   0x1FAFF,   "fonts/NotoEmoji-Regular.ttf"}   // Monochrome Emojis and Symbols
};

static FT_Library ft;
static std::map<std::string, FT_Face> loadedFaces;
static std::vector<Atlas> atlases;

const int ATLAS_WIDTH = 4096;
const int ATLAS_HEIGHT = 4096;
const int ATLAS_PADDING = 2;
const int FONT_SIZE = 24;

static FontRangeHash GetAtlasHash(const FontRange& range)
{
    FontRangeHash h;

    unsigned long long hashValue = 5381;

    hashValue = ((hashValue << 5) + hashValue) + range.start;
    hashValue = ((hashValue << 5) + hashValue) + range.end;

    for (char c : range.fontPath)
    {
        hashValue = ((hashValue << 5) + hashValue) + c;
    }

    std::stringstream ss;
    ss << std::hex << hashValue;
    std::string hexHash = ss.str();

    size_t len = std::min(hexHash.length(), sizeof(h.data) - 1);
    memcpy(h.data, hexHash.c_str(), len);
    h.data[len] = '\0';

    return h;
}

static void SerializeAtlas(Atlas& atlas, uint8_t* pixels, int width, int height)
{
    FontRangeHash atlasHash = GetAtlasHash(atlas.range);
    std::string directory = "fonts/cache/";
    std::string filename = directory + atlasHash.ToString() + ".sbatlas";

    std::filesystem::create_directories(directory);

    std::ofstream file(filename, std::ios::binary);

    if (!file)
    {
        SableUI_Error("Could not open file for writint cache: %s", filename.c_str());
        return;
    }

    // Header
    file.write(reinterpret_cast<const char*>(&atlas.range.start), sizeof(char32_t));
    file.write(reinterpret_cast<const char*>(&atlas.range.end), sizeof(char32_t));
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));

    // Main content
    size_t pxArraySize = static_cast<size_t>(width * height);
    file.write(reinterpret_cast<const char*>(pixels), pxArraySize * sizeof(uint8_t));

    file.close();
}

static bool DeserializeAtlas(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);

	if (!file)
	{
		SableUI_Error("Could not open file for reading atlas cache: %s", filename.c_str());
		return false;
	}
}

static FT_Face GetFontForChar(char32_t c)
{
    for (const auto& range : fontRanges)
    {
        if (c >= range.start && c <= range.end)
        {
            auto it = loadedFaces.find(range.fontPath);
            if (it != loadedFaces.end())
            {
                return it->second;
            }
        }
    }
    return nullptr;
}

static void RenderGlyphs(Atlas& atlas)
{
    atlas.characters.clear();

    int currentTempHeight = FONT_SIZE;
    uint8_t* tempPixels = new uint8_t[ATLAS_WIDTH * currentTempHeight];
    std::memset(tempPixels, 0, ATLAS_WIDTH * currentTempHeight * sizeof(uint8_t));

    int currentX = ATLAS_PADDING;
    int currentY = ATLAS_PADDING;
    int currentRowHeight = 0;

    for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
    {
        FT_Face face = GetFontForChar(c);
        if (!face) continue;

        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot glyph = face->glyph;
        int glyph_width = glyph->bitmap.width;
        int glyph_height = glyph->bitmap.rows;

        if (glyph_width == 0 || glyph_height == 0)
        {
            atlas.characters[c] = Character(
                atlas.textureID,
                SableUI::ivec2(glyph_width, glyph_height),
                SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
                static_cast<unsigned int>(glyph->advance.x >> 6),
                SableUI::vec4(0.0f, 0.0f, 0.0f, 0.0f)
            );
            continue;
        }

        if (currentX + glyph_width + ATLAS_PADDING > ATLAS_WIDTH)
        {
            currentX = ATLAS_PADDING;
            currentY += currentRowHeight + ATLAS_PADDING;
            currentRowHeight = 0;
        }

        if (currentY + glyph_height + ATLAS_PADDING > currentTempHeight)
        {
            int newTempHeight = std::min(ATLAS_HEIGHT, std::max(currentTempHeight * 2, currentY + glyph_height + ATLAS_PADDING));

            if (newTempHeight <= currentTempHeight)
            {
                SableUI_Error("FREETYPE: Texture atlas max height reached, or cannot allocate enough vertical space.");
                delete[] tempPixels;
                continue;
            }

            uint8_t* newPixels = new uint8_t[ATLAS_WIDTH * newTempHeight];
            std::memcpy(newPixels, tempPixels, ATLAS_WIDTH * currentTempHeight * sizeof(uint8_t));
            std::memset(newPixels + (ATLAS_WIDTH * currentTempHeight), 0, ATLAS_WIDTH * (newTempHeight - currentTempHeight) * sizeof(uint8_t));

            delete[] tempPixels;
            tempPixels = newPixels;
            currentTempHeight = newTempHeight;
        }

        currentRowHeight = std::max(currentRowHeight, glyph_height);

        for (int y = 0; y < glyph_height; ++y)
        {
            if (currentY + y < currentTempHeight)
            {
                for (int x = 0; x < glyph_width; ++x)
                {
                    if (currentX + x < ATLAS_WIDTH)
                    {
                        tempPixels[(currentY + y) * ATLAS_WIDTH + (currentX + x)] = glyph->bitmap.buffer[y * glyph_width + x];
                    }
                }
            }
        }

        float uv_x = (float)currentX / ATLAS_WIDTH;
        float uv_y = (float)currentY / currentTempHeight;
        float uv_w = (float)glyph_width / ATLAS_WIDTH;
        float uv_h = (float)glyph_height / currentTempHeight;

        Character character = {
            atlas.textureID,
            SableUI::ivec2(glyph_width, glyph_height),
            SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
            static_cast<unsigned int>(glyph->advance.x >> 6), // 1/64th of a pixel
            SableUI::vec4(uv_x, uv_y, uv_w, uv_h)
        };

        atlas.characters[c] = character;
        currentX += glyph_width + ATLAS_PADDING;
    }

    glGenTextures(1, &atlas.textureID);
    glBindTexture(GL_TEXTURE_2D, atlas.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, currentTempHeight, 0, GL_RED, GL_UNSIGNED_BYTE, tempPixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SerializeAtlas(atlas, tempPixels, ATLAS_WIDTH, currentTempHeight);

    delete[] tempPixels;
}

static void InitFreeType()
{
    if (FT_Init_FreeType(&ft))
    {
        SableUI_Runtime_Error("FREETYPE: Could not init FreeType library.");
        return;
    }

    std::vector<std::string> attemptedFontPaths;
    for (const auto& range : fontRanges)
    {
        if (std::find(attemptedFontPaths.begin(), attemptedFontPaths.end(),
            range.fontPath) != attemptedFontPaths.end()) continue;

        {
            FontRangeHash hash = GetAtlasHash(range);
			std::string directory = "fonts/cache/";
			std::string filename = directory + hash.ToString() + ".sbatlas";
			std::ifstream file(filename, std::ios::binary);
			if (file.is_open())
			{
                bool res = DeserializeAtlas(filename);
				file.close();
				if (res == true) continue;
			}
        }

        FT_Face face;
        if (FT_New_Face(ft, range.fontPath.c_str(), 0, &face))
        {
            SableUI_Error("FREETYPE: Could not load font: %s", range.fontPath.c_str());
            attemptedFontPaths.push_back(range.fontPath);
            continue;
        }
        FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
        loadedFaces[range.fontPath] = face;

        Atlas atlas;
        atlas.range = range;
        atlas.textureID = 0;
        atlases.push_back(atlas);

        RenderGlyphs(atlas);
    }

    if (loadedFaces.empty())
    {
        SableUI_Error("FREETYPE: No font faces were successfully loaded.");
    }
}

static void DestroyFreeType()
{
    for (auto& [_, face] : loadedFaces) {
        FT_Done_Face(face);
    }
    loadedFaces.clear();

    FT_Done_FreeType(ft);
}

void SableUI::Text::SetContent(const std::u32string& str)
{
    static bool isInitialized = false;
    if (!isInitialized)
    {
        InitFreeType();
        isInitialized = true;
        DestroyFreeType();
    }

    this->content = str;
}