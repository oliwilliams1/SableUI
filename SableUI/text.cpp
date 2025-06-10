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

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int ATLAS_PADDING = 2;
constexpr int FONT_SIZE = 24;

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

    ~Atlas() { if (textureID != 0) glDeleteTextures(1, &textureID); };
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

    const Atlas* GetAtlasForChar(char32_t c) const;

private:
    FontManager() : isInitialized(false) {}

    FT_Library ft_library = nullptr;
    std::map<std::string, FT_Face> loaded_faces_ft;
    std::vector<Atlas> atlases;

    const std::vector<FontRange> font_ranges = {
        {0x0000,    0x2FFF,    "fonts/NotoSans-Regular.ttf"},
        {0x2600,    0xD7A3,    "fonts/NotoSansKR-Regular.ttf"},
        {0x3040,    0x30FF,    "fonts/NotoSansJP-Regular.ttf"},
        {0x4E00,    0x9FFF,    "fonts/NotoSansSC-Regular.ttf"},
        {0x1F300,   0x1FAFF,   "fonts/NotoEmoji-Regular.ttf"}
    };

    FontRangeHash GetAtlasHash(const FontRange& range);
    void SerializeAtlas(Atlas& atlas, uint8_t* pixels, int width, int height);
    bool DeserializeAtlas(const std::string& filename, Atlas& out_atlas);
    FT_Face GetFontForChar(char32_t c);
    void RenderGlyphs(Atlas& atlas);
    void LoadAllFontRanges();
};

void SableUI::Text::SetContent(const std::u32string& str)
{
    if (!FontManager::GetInstance().isInitialized)
    {
        FontManager::GetInstance().Initialize();
    }

    this->content = str;
}

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

    atlases.clear();

    for (auto& [_, face] : loaded_faces_ft)
    {
        FT_Done_Face(face);
    }

    loaded_faces_ft.clear();

    FT_Done_FreeType(ft_library);
    SableUI_Log("FontManager shut down");
    isInitialized = false;
}

const Atlas* FontManager::GetAtlasForChar(char32_t c) const
{
    for (const auto& atlas : atlases) {
        if (c >= atlas.range.start && c <= atlas.range.end) {
            return &atlas;
        }
    }
    return nullptr;
}

FontRangeHash FontManager::GetAtlasHash(const FontRange& range)
{
    FontRangeHash h;
    unsigned long long hashValue = 5381;

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
    FontRangeHash atlasHash = GetAtlasHash(atlas.range);
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

bool FontManager::DeserializeAtlas(const std::string& filename, Atlas& out_atlas)
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

    file.read(reinterpret_cast<char*>(&start_char), sizeof(char32_t));
    file.read(reinterpret_cast<char*>(&end_char), sizeof(char32_t));
    file.read(reinterpret_cast<char*>(&path_len), sizeof(size_t));
    font_path_str.resize(path_len);
    file.read(font_path_str.data(), path_len);

    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    out_atlas.range.start = start_char;
    out_atlas.range.end = end_char;
    out_atlas.range.fontPath = font_path_str;
    out_atlas.isLoadedFromCache = true;

    size_t pxArraySize = static_cast<size_t>(width * height);
    uint8_t* pixels = new uint8_t[pxArraySize];
    file.read(reinterpret_cast<char*>(pixels), pxArraySize * sizeof(uint8_t));

    glGenTextures(1, &out_atlas.textureID);
    glBindTexture(GL_TEXTURE_2D, out_atlas.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
        character.textureID = out_atlas.textureID;
        out_atlas.characters[char_code] = character;
    }

    file.close();
    delete[] pixels;
    SableUI_Log("Loaded atlas from cache: %s", filename.c_str());
    return true;
}

FT_Face FontManager::GetFontForChar(char32_t c)
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

    auto it = loaded_faces_ft.find(target_font_path);
    if (it != loaded_faces_ft.end())
    {
        return it->second;
    }

    FT_Face face;
    if (FT_New_Face(ft_library, target_font_path.c_str(), 0, &face))
    {
        SableUI_Error("Could not load font: %s", target_font_path.c_str());
        return nullptr;
    }
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
    loaded_faces_ft[target_font_path] = face;
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

        if (currentY + glyph_height + ATLAS_PADDING > currentTempHeight)
        {
            int newTempHeight = std::min(ATLAS_HEIGHT, std::max(currentTempHeight * 2, currentY + glyph_height + ATLAS_PADDING));

            if (newTempHeight <= currentTempHeight)
            {
                SableUI_Error("Texture atlas max height reached. Cannot allocate enough vertical space.");
                delete[] tempPixels;
                return;
            }

            uint8_t* newPixels = new uint8_t[ATLAS_WIDTH * newTempHeight];
            std::memcpy(newPixels, tempPixels, ATLAS_WIDTH * currentTempHeight * sizeof(uint8_t));
            std::memset(newPixels + (ATLAS_WIDTH * currentTempHeight), 0, ATLAS_WIDTH * (newTempHeight - currentTempHeight) * sizeof(uint8_t));
            delete[] tempPixels;
            tempPixels = newPixels;
            currentTempHeight = newTempHeight;
        }

        currentRowHeight = std::max(currentRowHeight, glyph_height);

        for (int y = 0; y < glyph_height; y++)
        {
            if (currentY + y < currentTempHeight)
            {
                for (int x = 0; x < glyph_width; x++)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, currentTempHeight, 0, GL_RED, GL_UNSIGNED_BYTE, tempPixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (auto& pair : atlas.characters)
    {
        pair.second.textureID = atlas.textureID;
    }

    SerializeAtlas(atlas, tempPixels, ATLAS_WIDTH, currentTempHeight);
    delete[] tempPixels;
}

void FontManager::LoadAllFontRanges()
{
    for (const auto& range : font_ranges)
    {
        FontRangeHash hash = GetAtlasHash(range);
        std::string directory = "fonts/cache/";
        std::string filename = directory + hash.ToString() + ".sbatlas";

        Atlas current_atlas;
        bool loaded_from_cache = false;

        if (std::filesystem::exists(filename))
        {
            if (DeserializeAtlas(filename, current_atlas))
            {
                loaded_from_cache = true;
            }
            else
            {
                SableUI_Error("Failed to deserialize atlas from %s", filename.c_str());
            }
        }

        if (!loaded_from_cache) {
            current_atlas.range = range;
            RenderGlyphs(current_atlas);
        }

        atlases.emplace_back(current_atlas);
    }

    SableUI_Log("Loaded %zu font atlases.", atlases.size());
}
