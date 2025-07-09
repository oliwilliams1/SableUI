#include <filesystem>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <set> 

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SableUI/text.h"

#define SABLEUI_SUBSYSTEM "SableUI::Text"
#include "SableUI/console.h"

constexpr int ATLAS_WIDTH = 512;
constexpr int ATLAS_HEIGHT = 512;
constexpr int ATLAS_PADDING = 0;
constexpr int MIN_ATLAS_DEPTH = 1;
constexpr int MAX_ATLAS_GAP = 0;
constexpr const char* FONT_PREFIX = "f-";
constexpr char32_t MAX_CONTIGUOUS_CHARS_PER_RANGE = 256;

constexpr uint32_t ATLAS_CACHE_FILE_VERSION = 1;

struct FontRange {
	FontRange() = default;
	FontRange(char32_t start, char32_t end, std::string fontPath)
		: start(start), end(end), fontPath(fontPath) {}

	char32_t start = 0;
	char32_t end = 0;
	std::string fontPath = "";

	bool operator<(const FontRange& other) const
	{
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
	Character(SableUI::uvec2 pos, SableUI::uvec2 size, SableUI::ivec2 bearing,
		unsigned int advance, unsigned int layer)
		: pos(pos), size(size), bearing(bearing), advance(advance), layer(layer) {}

	SableUI::uvec2 pos = SableUI::uvec2(0);
	SableUI::uvec2 size = SableUI::uvec2(0);
	SableUI::ivec2 bearing = SableUI::ivec2(0);
	unsigned int advance = 0;
	unsigned int layer = 0;
};

struct Atlas {
	FontRange range;
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

	void ResizeTextureArray(int newDepth);
	GLuint GetAtlasTexture() const { return atlasTextureArray; }

private:
	FontManager() : isInitialized(false) {}

	void InitFreeType();
	void ShutdownFreeType();
	FT_Library ft_library = nullptr;

	bool FreeTypeRunning = false;

	std::vector<Atlas> atlases;
	std::map<char32_t, Character> characters;

	void FindFontRanges();
	bool FindFontRangeForChar(char32_t c, FontRange& outRange);
	std::vector<FontRange> cachedFontRanges;

	GLuint atlasTextureArray = 0;
	int atlasDepth = MIN_ATLAS_DEPTH;
	SableUI::uvec2 atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };

	std::set<std::pair<FontRange, int>> loadedAtlasKeys;

	const std::vector<FontRange> defaultFontRanges = {
		{ 0x0000, 0x00FF, "fonts/NotoSans-Regular.ttf" },
	};

	FontRangeHash GetAtlasHash(const FontRange& range, int fontSize);
	FT_Face GetFontForChar(char32_t c, int fontSize, const std::string& fontPathForAtlas,
		std::map<std::string, FT_Face>& currentLoadedFaces);
	void RenderGlyphs(Atlas& atlas);
	void LoadAllFontRanges();
	void LoadFontRange(Atlas& atlas, const FontRange& range);

	void SerialiseAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
		const std::map<char32_t, Character>& charsToSerialise);
	bool DeserialiseAtlas(const std::string& filename, Atlas& outAtlas);

	void UploadAtlasToGPU(int height, int initialY, uint8_t* pixels) const;
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

	/* check if font files are available */
	for (const auto& range : defaultFontRanges)
	{
		if (!std::filesystem::exists(range.fontPath))
		{
			SableUI_Runtime_Error("FontManager: Font file not found: %s", range.fontPath.c_str());
			return false;
		}
	}

	glGenTextures(1, &atlasTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTextureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, ATLAS_WIDTH, ATLAS_HEIGHT, MIN_ATLAS_DEPTH);

	// Sampling will be at fixed pixel sizes, GL_NEAREST will be used for sampling
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

	cachedFontRanges = defaultFontRanges;

	// iterate font/ dir and find .ttf and .otf files
	std::vector<std::filesystem::path> fontFiles;
	try {
		for (const auto& entry : std::filesystem::directory_iterator("fonts/"))
		{
			if (entry.path().extension() == ".ttf" || entry.path().extension() == ".otf")
			{
				bool isDefaultFont = false;
				for (const auto& defaultRange : defaultFontRanges) {
					if (defaultRange.fontPath == entry.path().string()) {
						isDefaultFont = true;
						break;
					}
				}
				if (!isDefaultFont)
				{
					fontFiles.push_back(entry.path());
				}
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
					if (charcode <= previousChar + MAX_ATLAS_GAP + 1 && contiguousCount < MAX_CONTIGUOUS_CHARS_PER_RANGE)
					{
						currentRange.end = charcode;
						contiguousCount++;
					}
					else
					{
						cachedFontRanges.push_back(currentRange);
						currentRange = { charcode, charcode, fontFile.string() };
						contiguousCount = 1; // Reset count for new range
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

	SableUI_Log("Resizing font atlas texture array from depth %i to %i", atlasDepth, newDepth);

	GLuint newTextureArray = 0;
	glGenTextures(1, &newTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, newTextureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, ATLAS_WIDTH, ATLAS_HEIGHT, newDepth);

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
	std::map<std::string, FT_Face>& currentLoadedFaces)
{
	if (fontPathForAtlas.empty())
	{
		SableUI_Error("GetFontForChar: Empty font path provided for char U+%04X", static_cast<unsigned int>(c));
		return nullptr;
	}

	auto it = currentLoadedFaces.find(fontPathForAtlas);
	if (it != currentLoadedFaces.end())
	{
		if (it->second->size->metrics.x_ppem != fontSize) {
			FT_Set_Pixel_Sizes(it->second, 0, fontSize);
		}
		return it->second;
	}

	FT_Face face;
	if (FT_New_Face(ft_library, fontPathForAtlas.c_str(), 0, &face))
	{
		SableUI_Error("Could not load font: %s for char U+%04X", fontPathForAtlas.c_str(), static_cast<unsigned int>(c));
		return nullptr;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
	currentLoadedFaces[fontPathForAtlas] = face;
	SableUI_Log("Loaded font: %s (size %i) for rendering atlas", fontPathForAtlas.c_str(), fontSize);
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
	unsigned int currentRowHeight = 0;
	int maxGlobalYReached = atlasCursor.y;

	for (char32_t c = atlas.range.start; c <= atlas.range.end; c++)
	{
		FT_Face face = GetFontForChar(c, atlas.fontSize, atlas.range.fontPath, facesForCurrentRender);
		if (!face) continue;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_BITMAP_METRICS_ONLY)) continue;

		FT_GlyphSlot glyph = face->glyph;
		SableUI::uvec2 size = { glyph->bitmap.width, glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0)
		{
			tempCursor.x += (glyph->advance.x >> 6) + ATLAS_PADDING;
			maxGlobalYReached = std::max(maxGlobalYReached, (int)tempCursor.y + (int)currentRowHeight);
			continue;
		}

		int currentLayer = tempCursor.y / ATLAS_HEIGHT;
		int potentialEndGlobalY = tempCursor.y + size.y;
		int currentLayerEndGlobalY = (currentLayer + 1) * ATLAS_HEIGHT;

		// Check for new line due to width overflow
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
			requiredHeightForPass = ATLAS_PADDING + 1; // Minimal height
		}
	}


	int proposedEndGlobalY = initialAtlasYForRenderPass + requiredHeightForPass;
	int newDepth = static_cast<int>(std::ceil(static_cast<float>(proposedEndGlobalY) / ATLAS_HEIGHT));

	if (newDepth > atlasDepth)
	{
		ResizeTextureArray(newDepth);
	}

	uint8_t* atlasPixels = new uint8_t[static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass];
	std::memset(atlasPixels, 0, static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass);

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
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER)) continue;

		FT_GlyphSlot glyph = face->glyph;
		SableUI::uvec2 size = { glyph->bitmap.width, glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0)
		{
			Character emptyChar = Character{
				atlasCursor,
				size,
				SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
				static_cast<unsigned int>(glyph->advance.x >> 6),
				static_cast<unsigned int>(atlasCursor.y / ATLAS_HEIGHT)
			};
			characters[c] = emptyChar;
			charsForSerialization[c] = emptyChar;

			atlasCursor.x += emptyChar.advance + ATLAS_PADDING;
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
			characters[c] = Character{
				atlasCursor,
				size,
				SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
				static_cast<unsigned int>(glyph->advance.x >> 6),
				static_cast<unsigned int>(atlasCursor.y / ATLAS_HEIGHT)
			};
			charsForSerialization[c] = characters[c];
			atlasCursor.x += size.x + ATLAS_PADDING;
			continue;
		}

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				size_t pixelIndexInAtlasPixels = static_cast<size_t>(yOffsetInAtlasPixels + y) * ATLAS_WIDTH + (atlasCursor.x + x);

				if (pixelIndexInAtlasPixels < static_cast<size_t>(ATLAS_WIDTH) * requiredHeightForPass)
				{
					if (glyph->bitmap.buffer)
					{
						atlasPixels[pixelIndexInAtlasPixels] = glyph->bitmap.buffer[static_cast<size_t>(y) * size.x + x];
					}
				}
			}
		}

		characters[c] = Character{
			atlasCursor,
			size,
			SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
			static_cast<unsigned int>(glyph->advance.x >> 6),
			static_cast<unsigned int>(atlasCursor.y / ATLAS_HEIGHT)
		};
		charsForSerialization[c] = characters[c];

		atlasCursor.x += size.x + ATLAS_PADDING;
	}

	UploadAtlasToGPU(requiredHeightForPass, initialAtlasYForRenderPass, atlasPixels);

	SerialiseAtlas(atlas, atlasPixels, ATLAS_WIDTH, requiredHeightForPass, charsForSerialization);

	delete[] atlasPixels;

	for (auto& pair : facesForCurrentRender)
	{
		FT_Done_Face(pair.second);
	}

	atlasCursor.y = initialAtlasYForRenderPass + requiredHeightForPass + ATLAS_PADDING;
}

void FontManager::UploadAtlasToGPU(int height, int initialY, uint8_t* pixels) const
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

		const uint8_t* chunkPixels = pixels + (static_cast<size_t>(currentUploadY) * ATLAS_WIDTH);

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, yOffsetInTargetLayer, targetLayer,
			ATLAS_WIDTH, heightToUpload, 1, GL_RED, GL_UNSIGNED_BYTE, chunkPixels);

		currentUploadY += heightToUpload;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void FontManager::SerialiseAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
	const std::map<char32_t, Character>& charsToSerialise)
{
	FontRangeHash atlasHash = GetAtlasHash(atlas.range, atlas.fontSize);
	std::string directory = "fonts/cache/";
	std::string filename = directory + FONT_PREFIX + atlasHash.ToString() + ".sbatlas";

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
	file.write(reinterpret_cast<const char*>(&atlas.fontSize), sizeof(int));
	file.write(reinterpret_cast<const char*>(&atlas.range.start), sizeof(char32_t));
	file.write(reinterpret_cast<const char*>(&atlas.range.end), sizeof(char32_t));
	size_t path_len = atlas.range.fontPath.length();
	file.write(reinterpret_cast<const char*>(&path_len), sizeof(size_t));
	file.write(atlas.range.fontPath.c_str(), path_len);

	file.write(reinterpret_cast<const char*>(&width), sizeof(int));
	file.write(reinterpret_cast<const char*>(&height), sizeof(int));

	/* main content */
	size_t pxArraySize = static_cast<size_t>(width) * height;
	file.write(reinterpret_cast<const char*>(pixels), pxArraySize * sizeof(uint8_t));

	size_t num_chars = charsToSerialise.size();
	file.write(reinterpret_cast<const char*>(&num_chars), sizeof(size_t));
	for (const auto& pair : charsToSerialise)
	{
		char32_t char_code = pair.first;
		const Character& character = pair.second;
		file.write(reinterpret_cast<const char*>(&char_code), sizeof(char32_t));
		file.write(reinterpret_cast<const char*>(&character.pos.x), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&character.pos.y), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&character.size.x), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&character.size.y), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&character.bearing.x), sizeof(int));
		file.write(reinterpret_cast<const char*>(&character.bearing.y), sizeof(int));
		file.write(reinterpret_cast<const char*>(&character.advance), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&character.layer), sizeof(unsigned int));
	}

	file.close();
	SableUI_Log("Saved atlas to %s", filename.c_str());
}

bool FontManager::DeserialiseAtlas(const std::string& filename, Atlas& out_atlas)
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

		file.read(reinterpret_cast<char*>(&out_atlas.fontSize), sizeof(int));
		file.read(reinterpret_cast<char*>(&out_atlas.range.start), sizeof(char32_t));
		file.read(reinterpret_cast<char*>(&out_atlas.range.end), sizeof(char32_t));
		size_t path_len{};
		file.read(reinterpret_cast<char*>(&path_len), sizeof(size_t));
		out_atlas.range.fontPath.resize(path_len);
		file.read(out_atlas.range.fontPath.data(), path_len);

		int cachedWidth{}, cachedHeight{};
		file.read(reinterpret_cast<char*>(&cachedWidth), sizeof(int));
		file.read(reinterpret_cast<char*>(&cachedHeight), sizeof(int));

		if (cachedWidth != ATLAS_WIDTH)
		{
			SableUI_Warn("Cached atlas width mismatch for %s. Expected %i, got %i. Regenerating.",
				filename.c_str(), ATLAS_WIDTH, cachedWidth);
			file.close();
			return false;
		}

		/* main content */
		size_t pxArraySize = static_cast<size_t>(cachedWidth) * cachedHeight;
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

		UploadAtlasToGPU(cachedHeight, initialAtlasYForDeserialisedPass, pixels_buffer.get());

		atlasCursor.y += cachedHeight + ATLAS_PADDING;

		size_t num_chars{};
		file.read(reinterpret_cast<char*>(&num_chars), sizeof(size_t));

		for (size_t i = 0; i < num_chars; ++i)
		{
			char32_t charCode{};
			Character character{};
			file.read(reinterpret_cast<char*>(&charCode), sizeof(char32_t));
			file.read(reinterpret_cast<char*>(&character.pos.x), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&character.pos.y), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&character.size.x), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&character.size.y), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&character.bearing.x), sizeof(int));
			file.read(reinterpret_cast<char*>(&character.bearing.y), sizeof(int));
			file.read(reinterpret_cast<char*>(&character.advance), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&character.layer), sizeof(unsigned int));

			characters[charCode] = character;
		}

		file.close();
		out_atlas.isLoadedFromCache = true;
		SableUI_Log("Deserialised atlas: %s", filename.c_str());
		return true;
	}
	catch (const std::exception& e)
	{
		SableUI_Error("Error during atlas deserialization from %s: %s",
			filename.c_str(), e.what());
		file.close();
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
	std::string directory = "fonts/cache/";
	std::string filename = directory + FONT_PREFIX + hash.ToString() + ".sbatlas";

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


struct Vertex
{
	SableUI::vec2 pos;
	SableUI::vec3 uv;
};

void FontManager::GetDrawInfo(SableUI::Text* text)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	SableUI::vec2 cursor = SableUI::vec2(0.0f, 0.0f);
	for (size_t i = 0; i < text->m_content.size(); i++)
	{
		const char32_t& c = text->m_content[i];
		Character char_data;

		auto it = characters.find(c);
		if (it != characters.end())
		{
			char_data = it->second;
		}
		else
		{
			FontRange targetRange;
			if (FindFontRangeForChar(c, targetRange))
			{
				Atlas newAtlas{};
				newAtlas.fontSize = text->m_fontSize;
				LoadFontRange(newAtlas, targetRange);

				it = characters.find(c);
				if (it != characters.end())
				{
					char_data = it->second;
				}
				else
				{
					SableUI_Warn("Could not find character U+%04X even after attempting to load its range. Using empty glyph.", c);
					char_data = Character{};
				}
			}
			else
			{
				SableUI_Warn("Could not find font range for character U+%04X. Using empty glyph.", c);
				char_data = Character{};
			}
		}

		float x = cursor.x + char_data.bearing.x;
		float y = cursor.y - char_data.bearing.y;

		float w = static_cast<float>(char_data.size.x);
		float h = static_cast<float>(char_data.size.y);

		float uBottomLeft = static_cast<float>(char_data.pos.x) / ATLAS_WIDTH;
		float vBottomLeft = static_cast<float>(char_data.pos.y) / ATLAS_HEIGHT;

		float uTopRight = uBottomLeft + (w / ATLAS_WIDTH);
		float vTopRight = vBottomLeft + (h / ATLAS_HEIGHT);

		float layerIndex = static_cast<float>(char_data.layer);

		vertices.push_back(Vertex{ {x, y},         {uBottomLeft, vBottomLeft - layerIndex, layerIndex} });
		vertices.push_back(Vertex{ {x + w, y},     {uTopRight,   vBottomLeft - layerIndex, layerIndex} });
		vertices.push_back(Vertex{ {x + w, y + h}, {uTopRight,   vTopRight - layerIndex,   layerIndex} });
		vertices.push_back(Vertex{ {x, y + h},     {uBottomLeft, vTopRight - layerIndex,   layerIndex} });

		unsigned int offset = static_cast<unsigned int>(i) * 4;
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);

		cursor.x += char_data.advance;
	}

	// Bind and upload vertex/index data
	glBindVertexArray(text->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text->m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	text->indiciesSize = indices.size();

	glBindVertexArray(0);
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
void SableUI::Text::SetContent(const std::u32string& str, int fontSize)
{
	// Ensure FontManager is initialized
	if (fontManager == nullptr || !fontManager->isInitialized)
	{
		FontManager::GetInstance().Initialize();
	}

	m_content = str;
	m_fontSize = fontSize;

	fontManager->GetDrawInfo(this);
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