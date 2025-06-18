#include <filesystem>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SableUI/text.h"
#include "SableUI/console.h"

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int ATLAS_PADDING = 2;
constexpr int MIN_ATLAS_DEPTH = 2;

struct FontRange {
	FontRange() = default;
	FontRange(char32_t start, char32_t end, std::string fontPath)
		: start(start), end(end), fontPath(fontPath) {}
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
	Character() = default;
	Character(SableUI::uvec2 size, SableUI::ivec2 bearing, unsigned int advance, SableUI::vec4 texCoords)
		: size(size), bearing(bearing), advance(advance), texCoords(texCoords) {
	};
	SableUI::ivec2 size;
	SableUI::ivec2 bearing;
	unsigned int advance;
	SableUI::vec4 texCoords;
};

struct Atlas {
	FontRange range;
	std::map<char32_t, Character> characters;
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
	void GetDrawInfo(SableUI::Text* text);
	const Atlas* GetAtlasForChar(char32_t c);

	void ResizeTextureArray(int newDepth);

private:
	FontManager() : isInitialized(false) {}

	void InitFreeType();
	void ShutdownFreeType();
	FT_Library ft_library = nullptr;

	bool FreeTypeRunning = false;

	std::vector<Atlas> atlases;

	GLuint atlasTextureArray = 0;
	int atlasDepth = MIN_ATLAS_DEPTH;
	SableUI::uvec2 atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };
	int currentDepth = 0;

	const std::vector<FontRange> font_ranges = {
		{0x0000,  0x2FFF,  "fonts/NotoSans-Regular.ttf"},
		{0xFB00,  0xFFFF,  "fonts/NotoSansJP-Regular.ttf"},
		{0x2600,  0xD7A3,  "fonts/NotoSansKR-Regular.ttf"},
		{0x3040,  0x30FF,  "fonts/NotoSansJP-Regular.ttf"},
		{0x4E00,  0x9FFF,  "fonts/NotoSansSC-Regular.ttf"},
		{0x1F300, 0x1FAFF, "fonts/NotoEmoji-Regular.ttf"}
	};

	FontRangeHash GetAtlasHash(const FontRange& range, int fontSize);
	FT_Face GetFontForChar(char32_t c, int fontSize, std::map<std::string, FT_Face>& currentLoadedFaces);
	void RenderGlyphs(Atlas& atlas);
	void LoadAllFontRanges();
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

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, ATLAS_WIDTH, ATLAS_HEIGHT, atlasDepth, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	LoadAllFontRanges();
	return true;
}

void FontManager::Shutdown()
{
	if (!isInitialized) return;

	glDeleteTextures(1, &atlasTextureArray);

	atlases.clear();

	ShutdownFreeType();
	SableUI_Log("FontManager shut down");
	isInitialized = false;
}

void FontManager::InitFreeType()
{
	SableUI_Log("Initializing FreeType");
	if (FT_Init_FreeType(&ft_library)) SableUI_Runtime_Error("Could not init FreeType library.");
	FreeTypeRunning = true;
}

void FontManager::ShutdownFreeType()
{
	SableUI_Log("Destroying FreeType");
	if (FreeTypeRunning)
	{
		FT_Done_FreeType(ft_library);
		FreeTypeRunning = false;
	}
}

const Atlas* FontManager::GetAtlasForChar(char32_t c)
{
	for (const auto& atlas : atlases)
	{
		if (c >= atlas.range.start && c <= atlas.range.end)
		{
			return &atlas;
		}
	}
	return nullptr;
}

void FontManager::ResizeTextureArray(int newDepth)
{
	if (newDepth <= atlasDepth)
	{
		SableUI_Warn("Trying to resize texture array to a smaller or same depth");
		return;
	}

	GLuint newTextureArray = 0;
	glGenTextures(1, &newTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, newTextureArray);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, ATLAS_WIDTH, ATLAS_HEIGHT, newDepth, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* copy old texture to new, width new depth */
	glCopyImageSubData(atlasTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, ATLAS_WIDTH, ATLAS_HEIGHT, atlasDepth);

	glDeleteTextures(1, &atlasTextureArray); // del old
	atlasTextureArray = newTextureArray;
	atlasDepth = newDepth;
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

FT_Face FontManager::GetFontForChar(char32_t c, int fontSize, std::map<std::string, FT_Face>& currentLoadedFaces)
{
	std::string targetFontPath;
	for (const auto& range : font_ranges)
	{
		if (c >= range.start && c <= range.end)
		{
			targetFontPath = range.fontPath;
			break;
		}
	}

	if (targetFontPath.empty()) return nullptr;

	auto it = currentLoadedFaces.find(targetFontPath);
	if (it != currentLoadedFaces.end())
	{
		return it->second;
	}

	FT_Face face;
	if (FT_New_Face(ft_library, targetFontPath.c_str(), 0, &face))
	{
		SableUI_Error("Could not load font: %s", targetFontPath.c_str());
		return nullptr;
	}
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	currentLoadedFaces[targetFontPath] = face;
	SableUI_Log("Loaded font: %s", targetFontPath.c_str());
	return face;
}

void FontManager::RenderGlyphs(Atlas& atlas)
{
	SableUI_Log("Rendering glyphs for range: U+%04X - U+%04X from %s",
		static_cast<unsigned int>(atlas.range.start),
		static_cast<unsigned int>(atlas.range.end),
		atlas.range.fontPath.c_str());

	atlas.characters.clear();

	std::map<std::string, FT_Face> facesForCurrentRender;

	int initialAtlasYForRenderPass = atlasCursor.y;

	/* get bounds of glyphs */
	SableUI::uvec2 tempCursor = { atlasCursor.x, atlasCursor.y };
	unsigned int currentRowHeight = 0;
	for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
	{
		FT_Face face = GetFontForChar(c, atlas.fontSize, facesForCurrentRender);
		if (!face) continue;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_BITMAP_METRICS_ONLY)) continue;

		FT_GlyphSlot glyph = face->glyph;
		SableUI::uvec2 size = { glyph->bitmap.width, glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0) continue;

		if (tempCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			tempCursor.x = ATLAS_PADDING;
			tempCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);
		tempCursor.x += size.x + ATLAS_PADDING;
	}

	/* add newline for the estimated size calculation */
	tempCursor.x = ATLAS_PADDING;
	tempCursor.y += currentRowHeight + ATLAS_PADDING;

	int requiredHeightForPass = tempCursor.y - initialAtlasYForRenderPass;

	if (initialAtlasYForRenderPass + requiredHeightForPass > atlasDepth * ATLAS_HEIGHT)
	{
		int newDepth = static_cast<int>(std::ceil(static_cast<float>(initialAtlasYForRenderPass + requiredHeightForPass) / ATLAS_HEIGHT));
		ResizeTextureArray(newDepth);
	}

	uint8_t* atlasPixels = new uint8_t[static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass];
	std::memset(atlasPixels, 0, static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass);

	currentRowHeight = 0;
	atlasCursor.x = ATLAS_PADDING;

	for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
	{
		FT_Face face = GetFontForChar(c, atlas.fontSize, facesForCurrentRender);
		if (!face) continue;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER)) continue;

		FT_GlyphSlot glyph = face->glyph;
		SableUI::uvec2 size = { glyph->bitmap.width, glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0)
		{
			atlas.characters[c] = Character{
				size,
				SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
				static_cast<unsigned int>(glyph->advance.x >> 6),
				SableUI::vec4(0.0f)
			};
			continue;
		}

		if (atlasCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			atlasCursor.x = ATLAS_PADDING;
			atlasCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);

		if ((atlasCursor.y - initialAtlasYForRenderPass + size.y) > requiredHeightForPass)
		{
			SableUI_Error("Atlas overflow during glyph rendering after resize attempt (glyph too tall for remaining space in pass)?!");
			break;
		}

		int yOffsetInAtlasPixels = (atlasCursor.y - initialAtlasYForRenderPass);

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				int pixelIndexInAtlasPixels = (yOffsetInAtlasPixels + y) * ATLAS_WIDTH + (atlasCursor.x + x);

				if (pixelIndexInAtlasPixels >= 0 && pixelIndexInAtlasPixels < ATLAS_WIDTH * requiredHeightForPass)
				{
					atlasPixels[pixelIndexInAtlasPixels] = glyph->bitmap.buffer[y * size.x + x];
				}
			}
		}

		SableUI::vec4 uv = SableUI::vec4(
			static_cast<float>(atlasCursor.x) / ATLAS_WIDTH,
			static_cast<float>(atlasCursor.y) / (atlasDepth * ATLAS_HEIGHT),
			static_cast<float>(size.x) / ATLAS_WIDTH,
			static_cast<float>(size.y) / (atlasDepth * ATLAS_HEIGHT)
		);

		atlas.characters[c] = Character{
			size,
			SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
			static_cast<unsigned int>(glyph->advance.x >> 6),
			uv
		};

		atlasCursor.x += size.x + ATLAS_PADDING;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTextureArray);

	int currentUploadY = 0;
	while (currentUploadY < requiredHeightForPass)
	{
		int globalYForChunk = initialAtlasYForRenderPass + currentUploadY;

		int targetLayer = globalYForChunk / ATLAS_HEIGHT;
		int yOffsetInTargetLayer = globalYForChunk % ATLAS_HEIGHT;

		int heightToUpload = std::min(ATLAS_HEIGHT - yOffsetInTargetLayer, requiredHeightForPass - currentUploadY);

		const uint8_t* chunkPixels = atlasPixels + (static_cast<size_t>(currentUploadY) * ATLAS_WIDTH);

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, yOffsetInTargetLayer, targetLayer,
			ATLAS_WIDTH, heightToUpload, 1, GL_RED, GL_UNSIGNED_BYTE, chunkPixels);

		currentUploadY += heightToUpload;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	delete[] atlasPixels;

	for (auto& pair : facesForCurrentRender)
	{
		FT_Done_Face(pair.second);
	}

	currentDepth = atlasCursor.y / ATLAS_HEIGHT;
}

void FontManager::LoadAllFontRanges()
{
	InitFreeType();

	for (const auto& range : font_ranges)
	{
		Atlas currentAtlas{};
		bool loaded_from_cache = false;
		currentAtlas.fontSize = 24;
		
		FontRangeHash hash = GetAtlasHash(range, currentAtlas.fontSize);
		std::string directory = "fonts/cache/";
		std::string filename = directory + hash.ToString() + ".sbatlas";

		/*if (std::filesystem::exists(filename))
		{
			if (DeserializeAtlas(filename, currentAtlas))
			{
				loaded_from_cache = true;
			}
			else
			{
				SableUI_Error("Failed to deserialize atlas from %s", filename.c_str());
			}
		}*/

		if (!loaded_from_cache) {
			currentAtlas.range = range;
			RenderGlyphs(currentAtlas);
		}

		atlases.emplace_back(currentAtlas);
	}

	SableUI_Log("Loaded %zu font atlases.", atlases.size());

	/* cleanup ft resources */
	ShutdownFreeType();
}

struct Vertex
{
	SableUI::vec2 pos;
	SableUI::vec2 uv;
};

void FontManager::GetDrawInfo(SableUI::Text* text)
{
	std::vector<Character> chars;

	for (const char32_t& c : text->m_content)
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

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	SableUI::vec2 cursor = SableUI::vec2(0.0f, 0.0f);
	for (size_t i = 0; i < chars.size(); i++)
	{
		float x = cursor.x + chars[i].bearing.x;
		float y = cursor.y - chars[i].bearing.y;

		float w = chars[i].size.x;
		float h = chars[i].size.y;

		float uBottomLeft = chars[i].texCoords.x;
		float vBottomLeft = chars[i].texCoords.y;
		float uTopRight = uBottomLeft + chars[i].texCoords.z;
		float vTopRight = vBottomLeft + chars[i].texCoords.w;

		vertices.push_back(Vertex{ {x,     y},     {uBottomLeft, vBottomLeft} });
		vertices.push_back(Vertex{ {x + w, y},     {uTopRight,   vBottomLeft} });
		vertices.push_back(Vertex{ {x + w, y + h}, {uTopRight,   vTopRight}   });
		vertices.push_back(Vertex{ {x,     y + h}, {uBottomLeft, vTopRight}   });

		unsigned int offset = static_cast<unsigned int>(i) * 4;
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);

		cursor.x += chars[i].advance;
	}

	glBindVertexArray(text->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text->m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	text->indiciesSize = indices.size();

	glBindVertexArray(0);
}

/* ------------- TEXT BACKEND ------------- */
void SableUI::Text::SetContent(const std::u32string& str, int fontSize)
{
	if (fontManager == nullptr)
	{
		FontManager::GetInstance().Initialize();
	}

	m_content  = str;
	m_fontSize = fontSize;

	// fontManager->GetDrawInfo(this);
}

void SableUI::InitFontManager()
{
	if (fontManager == nullptr)
	{
		FontManager::GetInstance().Initialize();
	}
}

void SableUI::DestroyFontManager()
{
	FontManager::GetInstance().Shutdown();
}

SableUI::Text::Text()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(SableUI::vec2)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

SableUI::Text::~Text()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}