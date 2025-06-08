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
#include <set>

static FT_Library ft;
static std::map<std::string, FT_Face> loadedFaces;

const int ATLAS_WIDTH = 6144;
const int ATLAS_HEIGHT = 6144;
const int ATLAS_PADDING = 2;
const int FONT_SIZE = 24;

struct FontRange {
    char32_t start;
    char32_t end;
    std::string fontPath;
};

std::vector<FontRange> fontRanges = {
    {0x0000,  0x00FF,  "fonts/NotoSans-Regular.ttf"},   // ASCII, Latin-1
    {0x0100,  0x017F,  "fonts/NotoSans-Regular.ttf"},   // Latin Extended
    {0x0370,  0x03FF,  "fonts/NotoSans-Regular.ttf"},   // Greek and Coptic
    {0x0400,  0x04FF,  "fonts/NotoSans-Regular.ttf"},   // Cyrillic
    {0x0600,  0x06FF,  "fonts/NotoSans-Regular.ttf"},   // Arabic
    {0x0900,  0x097F,  "fonts/NotoSans-Regular.ttf"},   // Devanagari
    {0x0E00,  0x0E7F,  "fonts/NotoSans-Regular.ttf"},   // Thai
    {0xAC00,  0xD7A3,  "fonts/NotoSansKR-Regular.ttf"}, // Hangul (Korean)
    {0x3040,  0x309F,  "fonts/NotoSansJP-Regular.ttf"}, // Hiragana (Japanese)
    {0x30A0,  0x30FF,  "fonts/NotoSansJP-Regular.ttf"}, // Katakana (Japanese)
    {0x4E00,  0x9FFF,  "fonts/NotoSansSC-Regular.ttf"}, // CJK (Simplified Chinese)
    {0x1F600, 0x1F64F, "fonts/NotoEmoji-Regular.ttf"},  // Basic Emojis
    {0x2600,  0x26FF,  "fonts/NotoSansKR-Regular.ttf"}, // Miscellaneous Symbols
    {0x20A0,  0x20CF,  "fonts/NotoSans-Regular.ttf"},   // Currency Symbols
    {0x1F300, 0x1F5FF, "fonts/NotoEmoji-Regular.ttf"},  // Miscellaneous Emoji
    {0x1F680, 0x1F6FF, "fonts/NotoEmoji-Regular.ttf"},  // Transport and Map Symbols
    {0x1F900, 0x1F9FF, "fonts/NotoEmoji-Regular.ttf"},  // Symbols and Pictographs
    {0x1FA00, 0x1FAFF, "fonts/NotoEmoji-Regular.ttf"},  // Supplemental Symbols and Pictographs
    {0x1F700, 0x1F77F, "fonts/NotoEmoji-Regular.ttf"},  // Alchemical Symbols
    {0x1F780, 0x1F7FF, "fonts/NotoEmoji-Regular.ttf"},  // Geometric Shapes Extended
};

struct Character {
    Character() = default;
    Character(GLuint tid, SableUI::ivec2 s, SableUI::ivec2 b, unsigned int a, SableUI::vec4 tc)
        : textureID(tid), size(s), bearing(b), advance(a), texCoords(tc) {
    }

    GLuint textureID;
    SableUI::ivec2 size;
    SableUI::ivec2 bearing;
    unsigned int advance;
    SableUI::vec4 texCoords;
};

std::map<char32_t, Character> characters;
std::vector<GLuint> atlasTextureIDs;

static void InitFreeType()
{
    if (FT_Init_FreeType(&ft))
    {
        SableUI_Runtime_Error("FREETYPE: Could not init FreeType library.");
        return;
    }

    std::set<std::string> attemptedFontPaths;
    for (const auto& range : fontRanges)
    {
        if (attemptedFontPaths.find(range.fontPath) == attemptedFontPaths.end())
        {
            if (loadedFaces.count(range.fontPath)) {
                attemptedFontPaths.insert(range.fontPath);
                continue;
            }

            FT_Face face;
            if (FT_New_Face(ft, range.fontPath.c_str(), 0, &face))
            {
                SableUI_Error("FREETYPE: Could not load font: %s", range.fontPath.c_str());
                attemptedFontPaths.insert(range.fontPath);
                continue;
            }
            FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
            loadedFaces[range.fontPath] = face;
            attemptedFontPaths.insert(range.fontPath);
        }
    }

    if (loadedFaces.empty())
    {
        SableUI_Error("FREETYPE: No font faces were successfully loaded.");
    }
}

static FT_Face GetFontForChar(char32_t c) {
    for (const auto& range : fontRanges) {
        if (c >= range.start && c <= range.end) {
            auto it = loadedFaces.find(range.fontPath);
            if (it != loadedFaces.end()) {
                return it->second;
            }
        }
    }
    return nullptr;
}

static void RenderGlyphs()
{
    characters.clear();
    for (GLuint id : atlasTextureIDs)
    {
        glDeleteTextures(1, &id);
    }
    atlasTextureIDs.clear();

    if (loadedFaces.empty())
    {
        SableUI_Error("FREETYPE: No font faces available to render glyphs. Call InitFreeType first.");
        return;
    }

    GLuint atlasID;
    glGenTextures(1, &atlasID);
    glBindTexture(GL_TEXTURE_2D, atlasID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    atlasTextureIDs.push_back(atlasID);

    int currentX = ATLAS_PADDING;
    int currentY = ATLAS_PADDING;
    int currentRowHeight = 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (char32_t c = 32; c < 0x20000; c++)
    {
        FT_Face face = GetFontForChar(c);
        if (!face) continue;

        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot glyph = face->glyph;
        int glyph_width = glyph->bitmap.width;
        int glyph_height = glyph->bitmap.rows;

        if (glyph_width == 0 || glyph_height == 0) {
            // store empty glypth for advance
            characters[c] = Character(
                atlasTextureIDs.back(),
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

        if (currentY + glyph_height + ATLAS_PADDING > ATLAS_HEIGHT)
        {
            SableUI_Error("FREETYPE: Texture atlas is full");
            break;
        }

        currentRowHeight = std::max(currentRowHeight, glyph_height);

        glBindTexture(GL_TEXTURE_2D, atlasTextureIDs.back());
        glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, glyph_width, glyph_height, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

        float uv_x = (float)currentX / ATLAS_WIDTH;
        float uv_y = (float)currentY / ATLAS_HEIGHT;
        float uv_w = (float)glyph_width / ATLAS_WIDTH;
        float uv_h = (float)glyph_height / ATLAS_HEIGHT;

        Character character = {
            atlasTextureIDs.back(),
            SableUI::ivec2(glyph_width, glyph_height),
            SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
            static_cast<unsigned int>(glyph->advance.x >> 6), // advance is 1/64 pixel
            SableUI::vec4(uv_x, uv_y, uv_w, uv_h)
        };

        characters[c] = character;
        currentX += glyph_width + ATLAS_PADDING;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

static void DestroyFreeType()
{
    for (auto& [_, face] : loadedFaces)
    {
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
        RenderGlyphs();
        isInitialized = true;
        DestroyFreeType();
    }

    this->content = str;
}