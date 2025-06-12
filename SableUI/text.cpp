#include <filesystem>
#include <map>
#include <algorithm>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SableUI/text.h"
#include "SableUI/console.h"

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int ATLAS_PADDING = 2;

struct FontRange {
    char32_t start = 0;
    char32_t end = 0;
    std::string fontPath = "";
};

struct FontRangeHash {
    char data[16] = { 0 };

    std::string ToString() const {
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
    bool isLoadedFromCache = false;
    int fontSize = 0;

    Atlas() = default;
};

class FontManager {
public:
    static FontManager& GetInstance();

    FontManager(FontManager const&) = delete;
    void operator=(FontManager const&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    bool Initialize();
    void Shutdown();
    bool isInitialized = false;
    static std::vector<SableUI::CharDrawInfo> GetDrawInfo(const std::u32string& str, float fontSize);
    static const Atlas* GetAtlasForChar(char32_t c);

private:
    FontManager() : isInitialized(false) {}

    FT_Library ft_library = nullptr;
    std::map<std::string, FT_Face> loadedFacesFT;
    static std::vector<Atlas> atlases;
    static std::vector<GLuint> atlasGLTextures;

    const std::vector<FontRange> font_ranges = {
        {0x0000,  0x2FFF,  "fonts/NotoSans-Regular.ttf"},
        {0xFB00,  0xFFFF,  "fonts/NotoSansJP-Regular.ttf"},
        {0x2600,  0xD7A3,  "fonts/NotoSansKR-Regular.ttf"},
        {0x3040,  0x30FF,  "fonts/NotoSansJP-Regular.ttf"},
        {0x4E00,  0x9FFF,  "fonts/NotoSansSC-Regular.ttf"},
        {0x1F300, 0x1FAFF, "fonts/NotoEmoji-Regular.ttf"}
    };

    FontRangeHash GetAtlasHash(const FontRange& range, int fontSize);
    void SerializeAtlas(Atlas& atlas, uint8_t* pixels, int width, int height);
    bool DeserializeAtlas(const std::string& filename, Atlas& out_atlas);
    FT_Face GetFontForChar(char32_t c, int fontSize);
    void RenderGlyphs(Atlas& atlas);
    void LoadAllFontRanges();
};

std::vector<Atlas> FontManager::atlases; // decl static member

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

    if (FT_Init_FreeType(&ft_library))
    {
        SableUI_Runtime_Error("Could not init FreeType library.");
        return false;
    }

    SableUI_Log("FontManager initialized");
    isInitialized = true;
    LoadAllFontRanges();
    return true;
}

void FontManager::Shutdown()
{
    if (!isInitialized) return;

    for (auto& atlas : atlases)
	{
		glDeleteTextures(1, &atlas.textureID);
	}

    atlases.clear();

    for (auto& [_, face] : loadedFacesFT)
    {
        FT_Done_Face(face);
    }

    loadedFacesFT.clear();

    FT_Done_FreeType(ft_library);
    SableUI_Log("FontManager shut down");
    isInitialized = false;
}

const Atlas* FontManager::GetAtlasForChar(char32_t c)
{
    for (const auto& atlas : atlases) {
        if (c >= atlas.range.start && c <= atlas.range.end) {
            return &atlas;
        }
    }
    return nullptr;
}

FontRangeHash FontManager::GetAtlasHash(const FontRange& range, int fontSize)
{
    FontRangeHash h;
    unsigned long long hashValue = 5381;

    hashValue = ((hashValue << 5) + hashValue) + fontSize;
    hashValue = ((hashValue << 5) + hashValue) + range.start;
    hashValue = ((hashValue << 5) + hashValue) + range.end;

    for (char c : range.fontPath) {
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

void FontManager::SerializeAtlas(Atlas& atlas, uint8_t* pixels, int width, int height)
{
    FontRangeHash atlasHash = GetAtlasHash(atlas.range, atlas.fontSize);
    std::string directory = "fonts/cache/";
    std::string filename = directory + atlasHash.ToString() + ".sbatlas";

    std::filesystem::create_directories(directory);

    std::ofstream file(filename, std::ios::binary);

    if (!file)
    {
        SableUI_Error("Could not open file for writing cache: %s", filename.c_str());
        return;
    }

    // Header
    file.write(reinterpret_cast<const char*>(&atlas.fontSize), sizeof(int));
    file.write(reinterpret_cast<const char*>(&atlas.range.start), sizeof(char32_t));
    file.write(reinterpret_cast<const char*>(&atlas.range.end), sizeof(char32_t));
    size_t path_len = atlas.range.fontPath.length();
    file.write(reinterpret_cast<const char*>(&path_len), sizeof(size_t));
    file.write(atlas.range.fontPath.c_str(), path_len);

    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));

    // Main content
    size_t pxArraySize = static_cast<size_t>(width * height);
    file.write(reinterpret_cast<const char*>(pixels), pxArraySize * sizeof(uint8_t));

    size_t num_chars = atlas.characters.size();
    file.write(reinterpret_cast<const char*>(&num_chars), sizeof(size_t));
    for (const auto& pair : atlas.characters)
    {
        char32_t char_code = pair.first;
        const Character& character = pair.second;
        file.write(reinterpret_cast<const char*>(&char_code), sizeof(char32_t));
        file.write(reinterpret_cast<const char*>(&character.size.x), sizeof(int));
        file.write(reinterpret_cast<const char*>(&character.size.y), sizeof(int));
        file.write(reinterpret_cast<const char*>(&character.bearing.x), sizeof(int));
        file.write(reinterpret_cast<const char*>(&character.bearing.y), sizeof(int));
        file.write(reinterpret_cast<const char*>(&character.advance), sizeof(unsigned int));
        file.write(reinterpret_cast<const char*>(&character.texCoords.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&character.texCoords.y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&character.texCoords.z), sizeof(float));
        file.write(reinterpret_cast<const char*>(&character.texCoords.w), sizeof(float));
    }

    file.close();
    SableUI_Log("Saved atlas to %s", filename.c_str());
}

bool FontManager::DeserializeAtlas(const std::string& filename, Atlas& outAtlas)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file)
    {
        SableUI_Error("Could not open cache file: %s", filename.c_str());
        return false;
    }

    int width = 0, height = 0;
    char32_t start_char = 0, end_char = 0;
    std::string font_path_str;
    size_t path_len = 0;

    file.read(reinterpret_cast<char*>(&outAtlas.fontSize), sizeof(int));
    file.read(reinterpret_cast<char*>(&start_char), sizeof(char32_t));
    file.read(reinterpret_cast<char*>(&end_char), sizeof(char32_t));
    file.read(reinterpret_cast<char*>(&path_len), sizeof(size_t));
    font_path_str.resize(path_len);
    file.read(font_path_str.data(), path_len);

    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    outAtlas.range.start = start_char;
    outAtlas.range.end = end_char;
    outAtlas.range.fontPath = font_path_str;
    outAtlas.isLoadedFromCache = true;

    size_t pxArraySize = static_cast<size_t>(width * height);
    uint8_t* pixels = new uint8_t[pxArraySize];
    file.read(reinterpret_cast<char*>(pixels), pxArraySize * sizeof(uint8_t));

    glGenTextures(1, &outAtlas.textureID);
    glBindTexture(GL_TEXTURE_2D, outAtlas.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    size_t num_chars = 0;
    file.read(reinterpret_cast<char*>(&num_chars), sizeof(size_t));
    for (size_t i = 0; i < num_chars; ++i)
    {
        char32_t char_code = 0;
        Character character{};
        file.read(reinterpret_cast<char*>(&char_code), sizeof(char32_t));
        file.read(reinterpret_cast<char*>(&character.size.x), sizeof(int));
        file.read(reinterpret_cast<char*>(&character.size.y), sizeof(int));
        file.read(reinterpret_cast<char*>(&character.bearing.x), sizeof(int));
        file.read(reinterpret_cast<char*>(&character.bearing.y), sizeof(int));
        file.read(reinterpret_cast<char*>(&character.advance), sizeof(unsigned int));
        file.read(reinterpret_cast<char*>(&character.texCoords.x), sizeof(float));
        file.read(reinterpret_cast<char*>(&character.texCoords.y), sizeof(float));
        file.read(reinterpret_cast<char*>(&character.texCoords.z), sizeof(float));
        file.read(reinterpret_cast<char*>(&character.texCoords.w), sizeof(float));
        character.textureID = outAtlas.textureID;
        outAtlas.characters[char_code] = character;
    }

    file.close();
    delete[] pixels;
    SableUI_Log("Loaded atlas from cache: %s", filename.c_str());
    return true;
}

FT_Face FontManager::GetFontForChar(char32_t c, int fontSize)
{
    std::string target_font_path;
    for (const auto& range : font_ranges)
    {
        if (c >= range.start && c <= range.end)
        {
            target_font_path = range.fontPath;
            break;
        }
    }

    if (target_font_path.empty()) return nullptr;

    auto it = loadedFacesFT.find(target_font_path);
    if (it != loadedFacesFT.end())
    {
        return it->second;
    }

    FT_Face face;
    if (FT_New_Face(ft_library, target_font_path.c_str(), 0, &face))
    {
        SableUI_Error("Could not load font: %s", target_font_path.c_str());
        return nullptr;
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    loadedFacesFT[target_font_path] = face;
    SableUI_Log("Loaded font: %s", target_font_path.c_str());
    return face;
}

void FontManager::RenderGlyphs(Atlas& atlas)
{
    SableUI_Log("Rendering glyphs for range: U+%04X - U+%04X from %s",
        static_cast<unsigned int>(atlas.range.start),
        static_cast<unsigned int>(atlas.range.end),
        atlas.range.fontPath.c_str());

    atlas.characters.clear();
    if (atlas.textureID != 0)
    {
        glDeleteTextures(1, &atlas.textureID);
        atlas.textureID = 0;
    }

    int currentTempHeight = atlas.fontSize;

    int max_current_y = ATLAS_PADDING;
    int current_x_calc = ATLAS_PADDING;
    int current_y_calc = ATLAS_PADDING;
    int current_row_height_calc = 0;

    for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
    {
        FT_Face face = GetFontForChar(c, atlas.fontSize);
        if (!face) continue;

        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_BITMAP_METRICS_ONLY)) continue;

        FT_GlyphSlot glyph = face->glyph;
        int glyph_width = glyph->bitmap.width;
        int glyph_height = glyph->bitmap.rows;

        if (glyph_width == 0 || glyph_height == 0) continue;

        if (current_x_calc + glyph_width + ATLAS_PADDING > ATLAS_WIDTH)
        {
            current_x_calc = ATLAS_PADDING;
            current_y_calc += current_row_height_calc + ATLAS_PADDING;
            current_row_height_calc = 0;
        }

        current_row_height_calc = std::max(current_row_height_calc, glyph_height);
        current_x_calc += glyph_width + ATLAS_PADDING;
        max_current_y = std::max(max_current_y, current_y_calc + current_row_height_calc + ATLAS_PADDING);
    }

    int final_atlas_height = std::min(ATLAS_HEIGHT, max_current_y);
    if (final_atlas_height < ATLAS_PADDING * 2) final_atlas_height = ATLAS_PADDING * 2;

    uint8_t* tempPixels = new uint8_t[ATLAS_WIDTH * final_atlas_height];
    std::memset(tempPixels, 0, ATLAS_WIDTH * final_atlas_height * sizeof(uint8_t));

    int currentX = ATLAS_PADDING;
    int currentY = ATLAS_PADDING;
    int currentRowHeight = 0;

    for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
    {
        FT_Face face = GetFontForChar(c, atlas.fontSize);
        if (!face) continue;

        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot glyph = face->glyph;
        int glyph_width = glyph->bitmap.width;
        int glyph_height = glyph->bitmap.rows;

        if (glyph_width == 0 || glyph_height == 0)
        {
            atlas.characters[c] = Character{
                atlas.textureID,
                SableUI::ivec2(glyph_width, glyph_height),
                SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
                static_cast<unsigned int>(glyph->advance.x >> 6),
                SableUI::vec4(0.0f, 0.0f, 0.0f, 0.0f)
            };
            continue;
        }

        if (currentX + glyph_width + ATLAS_PADDING > ATLAS_WIDTH)
        {
            currentX = ATLAS_PADDING;
            currentY += currentRowHeight + ATLAS_PADDING;
            currentRowHeight = 0;
        }

        currentRowHeight = std::max(currentRowHeight, glyph_height);

        if (currentY + glyph_height > final_atlas_height)
        {
            SableUI_Error("sums wrong");
            break;
        }


        for (int y = 0; y < glyph_height; y++)
        {
            for (int x = 0; x < glyph_width; x++)
            {
                if ((currentY + y) * ATLAS_WIDTH + (currentX + x) < ATLAS_WIDTH * final_atlas_height) {
                    tempPixels[(currentY + y) * ATLAS_WIDTH + (currentX + x)] = glyph->bitmap.buffer[y * glyph_width + x];
                }
            }
        }

        float uv_x = (float)currentX / ATLAS_WIDTH;
        float uv_y = (float)currentY / final_atlas_height;
        float uv_w = (float)glyph_width / ATLAS_WIDTH;
        float uv_h = (float)glyph_height / final_atlas_height;

        atlas.characters[c] = Character{
            atlas.textureID,
            SableUI::ivec2(glyph_width, glyph_height),
            SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
            static_cast<unsigned int>(glyph->advance.x >> 6),
            SableUI::vec4(uv_x, uv_y, uv_w, uv_h)
        };

        currentX += glyph_width + ATLAS_PADDING;
    }

    glGenTextures(1, &atlas.textureID);
    glBindTexture(GL_TEXTURE_2D, atlas.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, final_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, tempPixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (auto& pair : atlas.characters)
    {
        pair.second.textureID = atlas.textureID;
    }

    SerializeAtlas(atlas, tempPixels, ATLAS_WIDTH, final_atlas_height);
    delete[] tempPixels;
}

void FontManager::LoadAllFontRanges()
{
    for (const auto& range : font_ranges)
    {
        Atlas currentAtlas{};
        bool loaded_from_cache = false;
        currentAtlas.fontSize = 24;
        
        FontRangeHash hash = GetAtlasHash(range, currentAtlas.fontSize);
        std::string directory = "fonts/cache/";
        std::string filename = directory + hash.ToString() + ".sbatlas";

        if (std::filesystem::exists(filename))
        {
            if (DeserializeAtlas(filename, currentAtlas))
            {
                loaded_from_cache = true;
            }
            else
            {
                SableUI_Error("Failed to deserialize atlas from %s", filename.c_str());
            }
        }

        if (!loaded_from_cache) {
            currentAtlas.range = range;
            RenderGlyphs(currentAtlas);
        }

        atlases.emplace_back(currentAtlas);
    }

    SableUI_Log("Loaded %zu font atlases.", atlases.size());
}

std::vector<SableUI::CharDrawInfo> FontManager::GetDrawInfo(const std::u32string& str, float fontSize)
{
    std::vector<Character> chars;

    for (const char32_t& c : str)
    {
        const Atlas* atlas = GetAtlasForChar(c);
        if (atlas != nullptr) {
            auto it = atlas->characters.find(c);
            if (it != atlas->characters.end())
            {
                chars.emplace_back(it->second);
            }
            else {
                SableUI_Log("Could not find character U+%04X in atlas.", c);
            }
        }
    }

    std::vector<SableUI::CharDrawInfo> drawInfo(chars.size());

    SableUI::vec2 cursor = SableUI::vec2(0.0f, 0.0f);
    for (size_t i = 0; i < chars.size(); i++)
    {
        drawInfo[i].relPos = cursor;
        drawInfo[i].relPos.x += chars[i].bearing.x;
        drawInfo[i].relPos.y -= (chars[i].size.y - chars[i].bearing.y) + fontSize;
        drawInfo[i].size = chars[i].size;
        drawInfo[i].uv = chars[i].texCoords;
        drawInfo[i].atlasTextureID = chars[i].textureID; 
        cursor.x += chars[i].advance;
    }

    return drawInfo;
}

/* ------------- TEXT BACKEND ------------- */
void SableUI::Text::SetContent(const std::u32string& str, int fontSize)
{
    if (!FontManager::GetInstance().isInitialized)
    {
        FontManager::GetInstance().Initialize();
    }

    m_drawInfo.clear();
    m_content  = str;
    m_fontSize = fontSize;

    m_drawInfo = FontManager::GetDrawInfo(str, m_fontSize);
}

void SableUI::InitFontManager()
{
    if (!FontManager::GetInstance().isInitialized)
    {
        FontManager::GetInstance().Initialize();
    }
}

void SableUI::DestroyFontManager()
{
    FontManager::GetInstance().Shutdown();
}
