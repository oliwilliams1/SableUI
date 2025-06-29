#include <filesystem>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SableUI/text.h"
#include "SableUI/console.h"

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int ATLAS_PADDING = 1;
constexpr int MIN_ATLAS_DEPTH = 2;
constexpr int MAX_ATLAS_GAP = 64;
constexpr const char* FONT_PREFIX = "f-";

constexpr uint32_t ATLAS_CACHE_FILE_VERSION = 1;

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
	std::vector<FontRange> cachedFontRanges;

	GLuint atlasTextureArray = 0;
	int atlasDepth = MIN_ATLAS_DEPTH;
	SableUI::uvec2 atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };
	int currentDepth = 1;

	const std::vector<FontRange> font_ranges = {
		{ 0x0000, 0x00FF, "fonts/NotoSans-Regular.ttf" },
	};

	FontRangeHash GetAtlasHash(const FontRange& range, int fontSize);
	FT_Face GetFontForChar(char32_t c, int fontSize, std::map<std::string, FT_Face>& currentLoadedFaces);
	void RenderGlyphs(Atlas& atlas);
	void LoadAllFontRanges();

	void SerializeAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
		const std::map<char32_t, Character>& chars_to_serialize);
	bool DeserializeAtlas(const std::string& filename, Atlas& out_atlas);

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
	for (const auto& range : font_ranges)
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

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	atlasDepth = MIN_ATLAS_DEPTH;

	fontManager = &GetInstance();

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

	FindFontRanges();
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
	if (cachedFontRanges.size() > 0) return;

	// iterate font/ dir and find .ttf and .otf files
	std::vector<std::filesystem::path> fontFiles;

	for (const auto& entry : std::filesystem::directory_iterator("fonts/"))
	{
		if (entry.path().extension() == ".ttf" || entry.path().extension() == ".otf")
		{
			fontFiles.push_back(entry.path());
		}
	}

	// iterate through every char to see if font file has it, collect ranges
	for (const auto& fontFile : fontFiles)
	{
		FT_Face face;
		if (FT_New_Face(ft_library, fontFile.string().c_str(), 0, &face))
		{
			SableUI_Warn("Could not load font file: %s", fontFile.string().c_str());
			continue;
		}

		std::vector<FontRange> fontRanges;
		FT_ULong charcode = 0;
		FT_UInt glyphIndex = 0;

		charcode = FT_Get_First_Char(face, &glyphIndex);
		if (glyphIndex != 0)
		{
			FontRange currentRange = { charcode, charcode, fontFile.string() };
			FT_ULong previousChar = charcode;

			while (glyphIndex != 0)
			{
				charcode = FT_Get_Next_Char(face, charcode, &glyphIndex);

				if (glyphIndex != 0)
				{
					if (charcode <= previousChar + MAX_ATLAS_GAP + 1)
					{
						currentRange.end = charcode;
					}
					else
					{
						fontRanges.push_back(currentRange);
						currentRange = { charcode, charcode, fontFile.string() };
					}
					previousChar = charcode;
				}
			}
			fontRanges.push_back(currentRange);
		}

		FT_Done_Face(face);

		for (const auto& range : fontRanges)
		{
			if (range.end - range.start == 0) continue;
			cachedFontRanges.push_back(range);
		}
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

	glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTextureArray);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

FT_Face FontManager::GetFontForChar(char32_t c, int fontSize, std::map<std::string,
									FT_Face>& currentLoadedFaces)
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
		FT_Face face = GetFontForChar(c, atlas.fontSize, facesForCurrentRender);
		if (!face) continue;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
		if (glyphIndex == 0 || FT_Load_Glyph(face, glyphIndex, FT_LOAD_BITMAP_METRICS_ONLY)) continue;

		FT_GlyphSlot glyph = face->glyph;
		SableUI::uvec2 size = { glyph->bitmap.width, glyph->bitmap.rows };

		if (size.x == 0 || size.y == 0) continue;

		int potentialEndGlobalY = tempCursor.y + size.y;
		int currentLayerEndGlobalY = (tempCursor.y / ATLAS_HEIGHT + 1) * ATLAS_HEIGHT;

		if (potentialEndGlobalY > currentLayerEndGlobalY)
		{
			tempCursor.x = ATLAS_PADDING;
			tempCursor.y = (tempCursor.y / ATLAS_HEIGHT + 1) * ATLAS_HEIGHT + ATLAS_PADDING;
			currentRowHeight = 0;
		}
		else if (tempCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			tempCursor.x = ATLAS_PADDING;
			tempCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);
		tempCursor.x += size.x + ATLAS_PADDING;

		maxGlobalYReached = std::max(maxGlobalYReached, (int)tempCursor.y);
	}

	/* newline - final adjustment for the height calculation */
	if (currentRowHeight > 0 || tempCursor.x > ATLAS_PADDING)
	{
		maxGlobalYReached = std::max(maxGlobalYReached, 
			static_cast<int>(tempCursor.y + currentRowHeight + ATLAS_PADDING));
	}

	int requiredHeightForPass = maxGlobalYReached - initialAtlasYForRenderPass;

	if (requiredHeightForPass < 0) requiredHeightForPass = 0;
	if (requiredHeightForPass == 0 && (atlas.range.start <= atlas.range.end))
	{
		requiredHeightForPass = ATLAS_PADDING;
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

	std::map<char32_t, Character> chars_for_serialization;

	/* second pass, render glyphs and fill the cpu-side buffer */
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
			characters[c] = Character{
				atlasCursor,
				size,
				SableUI::ivec2(glyph->bitmap_left, glyph->bitmap_top),
				static_cast<unsigned int>(glyph->advance.x >> 6),
				static_cast<unsigned int>(atlasCursor.y / ATLAS_HEIGHT)
			};
			chars_for_serialization[c] = characters[c];
			continue;
		}

		int potentialEndGlobalY = atlasCursor.y + size.y;
		int currentLayerEndGlobalY = (atlasCursor.y / ATLAS_HEIGHT + 1) * ATLAS_HEIGHT;

		if (potentialEndGlobalY > currentLayerEndGlobalY)
		{
			atlasCursor.x = ATLAS_PADDING;
			atlasCursor.y = (atlasCursor.y / ATLAS_HEIGHT + 1) * ATLAS_HEIGHT + ATLAS_PADDING;
			currentRowHeight = 0;
		}
		else if (atlasCursor.x + size.x + ATLAS_PADDING > ATLAS_WIDTH)
		{
			atlasCursor.x = ATLAS_PADDING;
			atlasCursor.y += currentRowHeight + ATLAS_PADDING;
			currentRowHeight = 0;
		}

		currentRowHeight = std::max(currentRowHeight, size.y);

		if ((atlasCursor.y - initialAtlasYForRenderPass + size.y) > requiredHeightForPass)
		{
			SableUI_Error("Atlas overflow during glyph rendering after resize attempt (glyph too tall for remaining space in pass)! Char: U+%04X",
				static_cast<unsigned int>(c));
		}

		int yOffsetInAtlasPixels = (atlasCursor.y - initialAtlasYForRenderPass);

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				int pixelIndexInAtlasPixels = (yOffsetInAtlasPixels + y) * ATLAS_WIDTH + (atlasCursor.x + x);

				if (pixelIndexInAtlasPixels >= 0 && pixelIndexInAtlasPixels < ATLAS_WIDTH * requiredHeightForPass)
				{
					if (glyph->bitmap.buffer) {
						atlasPixels[pixelIndexInAtlasPixels] = glyph->bitmap.buffer[y * size.x + x];
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
		chars_for_serialization[c] = characters[c];

		atlasCursor.x += size.x + ATLAS_PADDING;
	}

	UploadAtlasToGPU(requiredHeightForPass, initialAtlasYForRenderPass, atlasPixels);

	SerializeAtlas(atlas, atlasPixels, ATLAS_WIDTH, requiredHeightForPass, chars_for_serialization);

	delete[] atlasPixels;

	for (auto& pair : facesForCurrentRender)
	{
		FT_Done_Face(pair.second);
	}

	atlasCursor.y = initialAtlasYForRenderPass + requiredHeightForPass + ATLAS_PADDING;

	currentDepth = static_cast<int>(std::ceil(static_cast<float>(atlasCursor.y) / ATLAS_HEIGHT));
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

void FontManager::SerializeAtlas(const Atlas& atlas, uint8_t* pixels, int width, int height,
	const std::map<char32_t, Character>& chars_to_serialize)
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
	size_t pxArraySize = static_cast<size_t>(width * height);
	file.write(reinterpret_cast<const char*>(pixels), pxArraySize * sizeof(uint8_t));

	size_t num_chars = chars_to_serialize.size();
	file.write(reinterpret_cast<const char*>(&num_chars), sizeof(size_t));
	for (const auto& pair : chars_to_serialize)
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

bool FontManager::DeserializeAtlas(const std::string& filename, Atlas& out_atlas)
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
		uint32_t file_version{};
		file.read(reinterpret_cast<char*>(&file_version), sizeof(uint32_t));
		if (file_version != ATLAS_CACHE_FILE_VERSION) {
			SableUI_Error("Atlas cache file version mismatch! File: %s. Expected %u, got %u. Regenerating.",
				filename.c_str(), ATLAS_CACHE_FILE_VERSION, file_version);
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
			SableUI_Error("Cached atlas width mismatch for %s. Expected %i, got %i. Regenerating.",
				filename.c_str(), ATLAS_WIDTH, cachedWidth);
			file.close();
			return false;
		}

		/* main content */
		size_t pxArraySize = static_cast<size_t>(cachedWidth * cachedHeight);
		std::unique_ptr<uint8_t[]> pixels_buffer = std::make_unique<uint8_t[]>(pxArraySize);
		file.read(reinterpret_cast<char*>(pixels_buffer.get()), pxArraySize * sizeof(uint8_t));

		if (!file)
		{
			SableUI_Error("Error reading pixel data from cache file: %s", filename.c_str());
			file.close();
			return false;
		}

		int initialAtlasYForDeserializedPass = atlasCursor.y;

		int proposedEndGlobalY = initialAtlasYForDeserializedPass + cachedHeight;
		int requiredDepthForDeserialization = static_cast<int>(std::ceil(static_cast<float>(proposedEndGlobalY) / ATLAS_HEIGHT));

		if (requiredDepthForDeserialization > atlasDepth)
		{
			ResizeTextureArray(requiredDepthForDeserialization);
		}


		UploadAtlasToGPU(cachedHeight, initialAtlasYForDeserializedPass, pixels_buffer.get());

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
		SableUI_Log("Deserialized atlas: %s", filename.c_str());
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

void FontManager::LoadAllFontRanges()
{
	InitFreeType();

	atlasCursor = { ATLAS_PADDING, ATLAS_PADDING };
	currentDepth = 0;
	characters.clear();


	for (const auto& range : font_ranges)
	{
		Atlas currentAtlas{};
		bool loaded_from_cache = false;
		currentAtlas.fontSize = 24;

		FontRangeHash hash = GetAtlasHash(range, currentAtlas.fontSize);
		std::string directory = "fonts/cache/";
		std::string filename = directory + FONT_PREFIX + hash.ToString() + ".sbatlas";

		if (std::filesystem::exists(filename))
		{
			if (DeserializeAtlas(filename, currentAtlas))
			{
				loaded_from_cache = true;
			}
			else
			{
				SableUI_Error("Failed to deserialize atlas from %s. Regenerating...",
					filename.c_str());
				loaded_from_cache = false;
			}
		}

		if (!loaded_from_cache) {
			currentAtlas.range = range;
			RenderGlyphs(currentAtlas);
		}

		atlases.emplace_back(currentAtlas);
	}

	SableUI_Log("Loaded %zu font atlases.", atlases.size());

	/* cleanup freetype resources */
	ShutdownFreeType();
}

struct Vertex
{
	SableUI::vec2 pos;
	SableUI::vec3 uv;
};

void FontManager::GetDrawInfo(SableUI::Text* text)
{
	std::vector<Character> chars_to_draw;

	for (const char32_t& c : text->m_content)
	{
		auto it = characters.find(c);
		if (it != characters.end())
		{
			chars_to_draw.emplace_back(it->second);
		}
		else {
			SableUI_Warn("Could not find character U+%04X in atlas. Using empty glyph.", c);
			chars_to_draw.emplace_back(Character{});
		}
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	SableUI::vec2 cursor = SableUI::vec2(0.0f, 0.0f);
	for (size_t i = 0; i < chars_to_draw.size(); i++)
	{
		const Character& char_data = chars_to_draw[i];

		float x = cursor.x + char_data.bearing.x;
		float y = cursor.y - char_data.bearing.y;

		float w = static_cast<float>(char_data.size.x);
		float h = static_cast<float>(char_data.size.y);

		float uBottomLeft = static_cast<float>(char_data.pos.x) / ATLAS_WIDTH;
		float vBottomLeft = static_cast<float>(char_data.pos.y) / ATLAS_HEIGHT;

		float uTopRight = uBottomLeft + (w / ATLAS_WIDTH);
		float vTopRight = vBottomLeft + (h / ATLAS_HEIGHT);

		float layerIndex = char_data.layer;

		vertices.push_back(Vertex{ {x,     y},     { uBottomLeft, vBottomLeft, layerIndex } });
		vertices.push_back(Vertex{ {x + w, y},     { uTopRight,   vBottomLeft, layerIndex } });
		vertices.push_back(Vertex{ {x + w, y + h}, { uTopRight,   vTopRight,   layerIndex } });
		vertices.push_back(Vertex{ {x,     y + h}, { uBottomLeft, vTopRight,   layerIndex } });

		unsigned int offset = static_cast<unsigned int>(i) * 4;
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);

		cursor.x += char_data.advance;
	}

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
	if (fontManager == nullptr)
	{
		FontManager::GetInstance().Initialize();
	}

	m_content  = str;
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