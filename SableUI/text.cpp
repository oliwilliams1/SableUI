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

#define SABLEUI_SUBSYSTEM "SableUI::Text"
#include "SableUI/console.h"

constexpr int ATLAS_WIDTH = 512;
constexpr int ATLAS_HEIGHT = 512;
constexpr int ATLAS_PADDING = 2;
constexpr int MIN_ATLAS_DEPTH = 1;
constexpr int MAX_ATLAS_GAP = 0;
constexpr const char* FONT_CACHE_PREFIX = "f-";
constexpr char32_t MAX_CONTIGUOUS_CHARS = 128;
constexpr uint32_t ATLAS_CACHE_FILE_VERSION = 1;

static SableUI::vec2 s_dpi = { 96.0f, 96.0f };
void SableUI::SetFontDPI(const vec2& dpi)
{
	s_dpi = dpi;
}

struct FontRange {
	FontRange() = default;
	FontRange(char32_t start, char32_t end, std::string fontPath)
		: start(start), end(end), fontPath(fontPath) {}

	char32_t start = 0;
	char32_t end = 0;
	std::string fontPath = "";

	bool operator<(const FontRange& other) const {
		if (start != other.start) return start < other.start;
		if (end != other.end) return end < other.end;
		return fontPath < other.fontPath;
	}
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
	FontRange range;
	bool isLoadedFromCache = false;
	int fontSize = 0;
	Atlas() = default;
};

struct char_t {
	char32_t c = 0;
	int fontSize = 0;

	bool operator<(const char_t& other) const {
		if (c != other.c) return c < other.c;
		return fontSize < other.fontSize;
	}
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
	int GetDrawInfo(SableUI::Text* text);

	void ResizeTextureArray(int newDepth);
	GLuint GetAtlasTexture() const { return atlasTextureArray; }

private:
	FontManager() : isInitialized(false) {}

	void InitFreeType();
	void ShutdownFreeType();
	FT_Library ft_library = nullptr;

	bool FreeTypeRunning = false;

	std::vector<Atlas> atlases;
	std::map<char_t, Character> characters;

	void FindFontRanges();
	bool FindFontRangeForChar(char32_t c, FontRange& outRange);
	std::vector<FontRange> cachedFontRanges;

	GLuint atlasTextureArray = 0;
	int atlasDepth = MIN_ATLAS_DEPTH;
	SableUI::u16vec2 atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };

	std::set<std::pair<FontRange, int>> loadedAtlasKeys;

	FontRangeHash GetAtlasHash(const FontRange& range, int fontSize);
	FT_Face GetFontForChar(char32_t c, int fontSize, const std::string& fontPathForAtlas,
		std::map<std::string, FT_Face>& currentLoadedFaces) const;
	void RenderGlyphs(Atlas& atlas);
	void LoadAllFontRanges();
	void LoadFontRange(Atlas& atlas, const FontRange& range);

	void SerialiseAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
		const std::map<char32_t, Character>& charsToSerialise, GLenum pixelType, int initialYOffset);
	bool DeserialiseAtlas(const std::string& filename, Atlas& outAtlas);

	void UploadAtlasToGPU(int height, int initialY, uint8_t* pixels, GLenum pixelType) const;
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
	FindFontRanges();
	LoadAllFontRanges();
	return true;
}

void FontManager::Shutdown()
{
	if (!isInitialized) return;

	glDeleteTextures(1, &atlasTextureArray);
	atlasTextureArray = 0;

	atlases.clear();
	characters.clear();

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

void FontManager::FindFontRanges()
{
	if (!FreeTypeRunning) SableUI_Runtime_Error("FreeType is not running");

	if (!cachedFontRanges.empty())
	{
		SableUI_Log("Font ranges already cached");
		return;
	}

	// iterate font/ dir and find .ttf and .otf files
	std::vector<std::filesystem::path> fontFiles;
	try {
		for (const auto& entry : std::filesystem::directory_iterator("fonts/"))
		{
			if (entry.path().extension() == ".ttf" || entry.path().extension() == ".otf")
			{
				fontFiles.push_back(entry.path());
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		SableUI_Error("FontManager: Could not iterate 'fonts/' directory: %s", e.what());
	}

	// iterate through every char to see if font file has it, collect ranges
	for (const auto& fontFile : fontFiles)
	{
		FT_Face face;
		if (FT_New_Face(ft_library, fontFile.string().c_str(), 0, &face))
		{
			SableUI_Warn("Could not load font file for range discovery: %s", fontFile.string().c_str());
			continue;
		}

		FT_Set_Pixel_Sizes(face, 0, 12);
		FT_Set_Char_Size(face, 0, 12 * 64, s_dpi.x, s_dpi.y);

		FT_ULong charcode = 0;
		FT_UInt glyphIndex = 0;

		charcode = FT_Get_First_Char(face, &glyphIndex);
		if (glyphIndex != 0)
		{
			FontRange currentRange = { charcode, charcode, fontFile.string() };
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
						cachedFontRanges.push_back(currentRange);
						currentRange = { charcode, charcode, fontFile.string() };
						contiguousCount = 1;
					}
					previousChar = charcode;
				}
			}
			cachedFontRanges.push_back(currentRange);
		}

		FT_Done_Face(face);
	}
	SableUI_Log("Discovered %zu total font ranges.", cachedFontRanges.size());
}

bool FontManager::FindFontRangeForChar(char32_t c, FontRange& outRange)
{
	for (const auto& range : cachedFontRanges)
	{
		if (c >= range.start && c <= range.end)
		{
			outRange = range;
			return true;
		}
	}
	return false;
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

FontRangeHash FontManager::GetAtlasHash(const FontRange& range, int fontSize)
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
		else {
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

			char_t ct = { c, atlas.fontSize };
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

			char_t ct = { c, atlas.fontSize };
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

		char_t ct = { c, atlas.fontSize };
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

	try {
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
		{
			ResizeTextureArray(requiredDepthForDeserialization);
		}

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

			char_t key = { charCode, outAtlas.fontSize };
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

void FontManager::LoadFontRange(Atlas& atlas, const FontRange& range)
{
	atlas.range = range;

	std::pair<FontRange, int> currentAtlasKey = { range, atlas.fontSize };
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
			SableUI_Warn("Failed to deserialise atlas from %s. Regenerating...",
				filename.c_str());
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

void FontManager::LoadAllFontRanges()
{
	SableUI_Log("Preparing FontManager for on-demand atlas loading.");

	atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };
	atlasDepth = MIN_ATLAS_DEPTH;

	characters.clear();
	atlases.clear();
	loadedAtlasKeys.clear();

	if (!FreeTypeRunning) InitFreeType();
	FindFontRanges();
	SableUI_Log("FontManager ready for character requests.");
}

struct Vertex {
	SableUI::vec2 pos;
	SableUI::vec3 uv; // uv.x and uv.y for texture coordinates, uv.z for layer
};

struct TextToken {
	std::u32string content = U"";
	float width = 0.0f;
	bool isSpace = false;
	bool isNewline = false;
	std::vector<Character> charDataList;
};

int FontManager::GetDrawInfo(SableUI::Text* text)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int height = 0;

	std::vector<TextToken> tokens;
	TextToken currentToken;

	/* wordwrapping */
	for (char32_t c : text->m_content)
	{
		if (c == U'\n') {
			if (!currentToken.content.empty()) {
				tokens.push_back(currentToken);
			}
			TextToken newlineToken;
			newlineToken.isNewline = true;
			tokens.push_back(newlineToken);
			currentToken = TextToken{};
			continue;
		}

		Character charData;
		char_t charKey = { c, text->m_fontSize };
		auto it = characters.find(charKey);
		if (it != characters.end())
		{
			charData = it->second;
		}
		else
		{
			FontRange targetRange;
			if (FindFontRangeForChar(c, targetRange))
			{
				Atlas newAtlas{};
				newAtlas.fontSize = text->m_fontSize;
				LoadFontRange(newAtlas, targetRange);

				it = characters.find(charKey);
				if (it != characters.end())
				{
					charData = it->second;
				}
				else
				{
					SableUI_Warn("Could not find character U+%04X with font size %d even after attempting to load its range. Using empty glyph.",
						c, text->m_fontSize);
					charData = Character{};
				}
			}
			else
			{
				SableUI_Warn("Could not find font range for character U+%04X. Using empty glyph.", c);
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
			{
				currentToken.isSpace = true;
			}
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
	{
		tokens.push_back(currentToken);
	}

	SableUI::vec2 cursor{};

	/* calc top left position via line height */
	{
		int tempLines = 1;
		SableUI::vec2 tempCursor{};
		float tempCurrentLineX = 0.0f;

		for (const TextToken& token : tokens)
		{
			if (token.isNewline)
			{
				tempCursor.x = 0;
				tempCursor.y += text->m_lineSpacingPx;
				tempLines++;
				tempCurrentLineX = 0.0f;
				continue;
			}

			if (!token.isSpace && text->m_maxWidth > 0
				&& (tempCurrentLineX + token.width > text->m_maxWidth) && tempCurrentLineX > 0)
			{
				tempCursor.x = 0;
				tempCursor.y += text->m_lineSpacingPx;
				tempLines++;
				tempCurrentLineX = 0.0f;
			}

			for (size_t i = 0; i < token.content.length(); i++)
			{
				const Character& charData = token.charDataList[i];
				tempCursor.x += charData.advance;
				tempCurrentLineX += charData.advance;
			}
		}

		tempLines;
		height = tempLines * text->m_lineSpacingPx;
		cursor.y -= height;
	}

	float currentLineX = 0.0f;
	unsigned int currentGlyphOffset = 0;
	int lines = 1;

	for (const TextToken& token : tokens)
	{
		if (token.isNewline)
		{
			cursor.x = 0;
			cursor.y += text->m_lineSpacingPx;
			lines++;
			currentLineX = 0.0f;
			continue;
		}

		if (!token.isSpace && text->m_maxWidth > 0
			&& (currentLineX + token.width > text->m_maxWidth) && currentLineX > 0)
		{
			cursor.x = 0;
			cursor.y += text->m_lineSpacingPx;
			lines++;
			currentLineX = 0.0f;
		}

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

			vertices.push_back(Vertex{ {x, y},           {uBottomLeft, vBottomLeft, layerIndex} });
			vertices.push_back(Vertex{ {x + w, y},       {uTopRight,   vBottomLeft, layerIndex} });
			vertices.push_back(Vertex{ {x + w, y + h},   {uTopRight,   vTopRight,   layerIndex} });
			vertices.push_back(Vertex{ {x, y + h},       {uBottomLeft, vTopRight,   layerIndex} });

			unsigned int offset = currentGlyphOffset * 4;
			indices.push_back(offset);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			indices.push_back(offset);
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);

			cursor.x += charData.advance;
			currentLineX += charData.advance;
			currentGlyphOffset++;
		}
	}

	glBindVertexArray(text->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text->m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
				 indices.data(), GL_STATIC_DRAW);

	text->indiciesSize = indices.size();

	glBindVertexArray(0);

	return height;
}

GLuint SableUI::GetAtlasTexture()
{
	if (!FontManager::GetInstance().isInitialized)
	{
		FontManager::GetInstance().Initialize();
	}
	return FontManager::GetInstance().GetAtlasTexture();
}

/* ------------- TEXT BACKEND ------------- */
int SableUI::Text::SetContent(const SableString& str, int maxWidth, int fontSize, float lineSpacingFac)
{
	// Ensure FontManager is initialized
	if (fontManager == nullptr || !fontManager->isInitialized)
	{
		FontManager::GetInstance().Initialize();
	}

	m_content = str;
	m_fontSize = fontSize;
	m_lineSpacingPx = fontSize * lineSpacingFac;
	m_maxWidth = maxWidth;

	int height = fontManager->GetDrawInfo(this);
	return height;
}

int SableUI::Text::UpdateMaxWidth(int maxWidth)
{
	// Ensure FontManager is initialized
	if (fontManager == nullptr || !fontManager->isInitialized)
	{
		FontManager::GetInstance().Initialize();
	}

	m_maxWidth = maxWidth;

	int height = fontManager->GetDrawInfo(this);
	return height;
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

	// Position attribute (layout 0, 2 floats)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// UV + Layer attribute (layout 1, 3 floats)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(SableUI::vec2)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

SableUI::Text::~Text()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	m_VAO = 0; m_VBO = 0; m_EBO = 0;
}