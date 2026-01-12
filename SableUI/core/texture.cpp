#include <SableUI/core/texture.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component.h>
#include <SableUI/core/renderer.h>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Texture"

#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <webp/decode.h>
#include <algorithm>
#include <iterator>
#include <mutex>
#include <queue>
#include <thread>

using namespace SableUI;

struct ImageHash
{
	char data[16] = { 0 };

	bool operator<(const ImageHash& other) const
	{
		return std::memcmp(data, other.data, sizeof(data)) < 0;
	}
};

struct SableUI::CachedGpuTexture
{
	uint8_t* cpuData = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	GpuTexture2D gpuTexture;

	TextureLoadState state = TextureLoadState::Empty;
	bool inUse = true;
	std::chrono::steady_clock::time_point lastUsed = std::chrono::steady_clock::now();

	std::mutex dataMutex;

	std::vector<BaseComponent*> dependentComponents;
	std::mutex componentsMutex;

	~CachedGpuTexture();
};

static std::map<ImageHash, std::shared_ptr<CachedGpuTexture>> textureCache;
static int s_numTextures = 0;

static ImageHash GetImageHash(const std::string& path, int width, int height)
{
	unsigned long long hashValue = 5381;
	hashValue = ((hashValue << 5) + hashValue) + width;
	hashValue = ((hashValue << 5) + hashValue) + height;

	for (char c : path)
		hashValue = ((hashValue << 5) + hashValue) + static_cast<unsigned long long>(c);

	ImageHash h;
	std::stringstream ss;
	ss << std::hex << hashValue;
	std::string hexHash = ss.str();

	size_t len = std::min(hexHash.length(), sizeof(h.data) - 1);
	memcpy(h.data, hexHash.c_str(), len);
	h.data[len] = '\0';
	return h;
}

// ============================================================================
// CachedGpuTexture
// ============================================================================
CachedGpuTexture::~CachedGpuTexture()
{
	std::lock_guard<std::mutex> lock(dataMutex);

	if (cpuData)
	{
		stbi_image_free(cpuData);
		cpuData = nullptr;
	}
}

// ============================================================================
// AsyncTextureLoader
// ============================================================================
AsyncTextureLoader& AsyncTextureLoader::GetInstance()
{
	static AsyncTextureLoader instance;
	return instance;
}

AsyncTextureLoader::~AsyncTextureLoader()
{
	Shutdown();
}

void AsyncTextureLoader::Initialise()
{
	if (m_running.load())
		return;

	m_running.store(true);
	m_worker = std::thread(&AsyncTextureLoader::WorkerThread, this);

	SableUI_Log("Async texture loader initialized");
}

void AsyncTextureLoader::Shutdown()
{
	if (!m_running.load())
		return;

	m_running.store(false);
	m_queueCV.notify_one();

	if (m_worker.joinable())
		m_worker.join();

	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		while (!m_loadQueue.empty())
			m_loadQueue.pop();
	}

	{
		std::lock_guard<std::mutex> lock(m_completedMutex);
		while (!m_completedQueue.empty())
			m_completedQueue.pop();
	}

	SableUI_Log("Async texture loader shut down");
}

void AsyncTextureLoader::QueueLoad(const std::string& path, int width, int height,
	std::shared_ptr<CachedGpuTexture> target)
{
	if (!m_running.load())
	{
		SableUI_Warn("Async texture loader not initialized");
		return;
	}

	TextureLoadRequest request;
	request.path = path;
	request.targetWidth = width;
	request.targetHeight = height;
	request.target = target;

	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_loadQueue.push(request);
	}

	m_queueCV.notify_one();
}

void AsyncTextureLoader::ProcessCompletedLoads()
{
	std::queue<TextureLoadRequest> toProcess;

	{
		std::lock_guard<std::mutex> lock(m_completedMutex);
		toProcess.swap(m_completedQueue);
	}

	while (!toProcess.empty())
	{
		TextureLoadRequest& req = toProcess.front();

		if (req.target && req.target->state == TextureLoadState::Loaded)
		{
			std::lock_guard<std::mutex> dataLock(req.target->dataMutex);

			if (req.target->cpuData && req.target->width > 0 && req.target->height > 0)
			{
				TextureFormat format = TextureFormat::RGB8;
				if (req.target->channels == 4)
					format = TextureFormat::RGBA8;
				else if (req.target->channels == 1)
					format = TextureFormat::R8;
				else if (req.target->channels == 2)
					format = TextureFormat::RG8;

				req.target->gpuTexture.SetData(
					req.target->cpuData,
					req.target->width,
					req.target->height,
					format
				);

				// Free CPU memory
				stbi_image_free(req.target->cpuData);
				req.target->cpuData = nullptr;

				req.target->state = TextureLoadState::Uploaded;

				SableUI_Log("Uploaded texture to GPU: %s (%dx%d)",
					req.path.c_str(), req.target->width, req.target->height);
			}
			else
			{
				req.target->state = TextureLoadState::Failed;
			}
		}

		if (req.target)
		{
			std::lock_guard<std::mutex> compLock(req.target->componentsMutex);
			for (BaseComponent* comp : req.target->dependentComponents)
			{
				if (comp)
					comp->needsRerender = true;
			}
		}

		toProcess.pop();
	}
}

void AsyncTextureLoader::WorkerThread()
{
	while (m_running.load())
	{
		TextureLoadRequest request;

		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_queueCV.wait(lock, [this] {
				return !m_loadQueue.empty() || !m_running.load();
				});

			if (!m_running.load())
				break;

			if (m_loadQueue.empty())
				continue;

			request = m_loadQueue.front();
			m_loadQueue.pop();
		}

		if (!request.target)
			continue;

		int loadedWidth, loadedHeight, loadedChannels;
		uint8_t* loadedPixels = nullptr;

		loadedPixels = stbi_load(request.path.c_str(), &loadedWidth, &loadedHeight,
			&loadedChannels, 0);

		if (!loadedPixels)
		{
			std::ifstream file(request.path, std::ios::binary);
			if (file.is_open())
			{
				std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
					std::istreambuf_iterator<char>());
				file.close();

				if (buffer.size() >= 12 &&
					memcmp(buffer.data(), "RIFF", 4) == 0 &&
					memcmp(buffer.data() + 8, "WEBP", 4) == 0)
				{
					loadedPixels = WebPDecodeRGB(buffer.data(), buffer.size(),
						&loadedWidth, &loadedHeight);
					loadedChannels = 3;
				}
			}
		}

		if (!loadedPixels)
		{
			SableUI_Warn("Failed to load texture: %s", request.path.c_str());
			request.target->state = TextureLoadState::Failed;
			continue;
		}

		const int targetChannels = 3;
		uint8_t* processedPixels = nullptr;
		int finalWidth = loadedWidth;
		int finalHeight = loadedHeight;

		if (loadedChannels != targetChannels)
		{
			processedPixels = new uint8_t[loadedWidth * loadedHeight * targetChannels];

			for (int i = 0; i < loadedWidth * loadedHeight; i++)
			{
				if (loadedChannels == 4)
				{
					processedPixels[i * 3 + 0] = loadedPixels[i * 4 + 0];
					processedPixels[i * 3 + 1] = loadedPixels[i * 4 + 1];
					processedPixels[i * 3 + 2] = loadedPixels[i * 4 + 2];
				}
				else if (loadedChannels == 1)
				{
					processedPixels[i * 3 + 0] =
						processedPixels[i * 3 + 1] =
						processedPixels[i * 3 + 2] = loadedPixels[i];
				}
			}

			stbi_image_free(loadedPixels);
			loadedPixels = processedPixels;
			loadedChannels = targetChannels;
		}

		if (request.targetWidth > 0 && request.targetHeight > 0 &&
			(request.targetWidth < loadedWidth || request.targetHeight < loadedHeight))
		{
			uint8_t* resizedPixels = new uint8_t[request.targetWidth * request.targetHeight * targetChannels];

			bool success = stbir_resize_uint8_linear(
				loadedPixels, loadedWidth, loadedHeight, 0,
				resizedPixels, request.targetWidth, request.targetHeight, 0,
				STBIR_RGB
			) != nullptr;

			if (success)
			{
				if (processedPixels)
					delete[] processedPixels;
				else
					stbi_image_free(loadedPixels);

				loadedPixels = resizedPixels;
				finalWidth = request.targetWidth;
				finalHeight = request.targetHeight;
			}
			else
			{
				delete[] resizedPixels;
			}
		}

		{
			std::lock_guard<std::mutex> lock(request.target->dataMutex);
			request.target->cpuData = loadedPixels;
			request.target->width = finalWidth;
			request.target->height = finalHeight;
			request.target->channels = loadedChannels;
			request.target->state = TextureLoadState::Loaded;
		}

		{
			std::lock_guard<std::mutex> lock(m_completedMutex);
			m_completedQueue.push(request);
		}

		PostEmptyEvent();
	}
}

// ============================================================================
// Texture
// ============================================================================
Texture::Texture()
{
	s_numTextures++;
}

Texture::Texture(int width, int height, uint32_t texID)
	: m_width(width), m_height(height)
{
	s_numTextures++;
	m_defaultTexID = texID;
}

Texture::~Texture()
{
	if (m_cachedGpu)
	{
		m_cachedGpu->inUse = false;
		m_cachedGpu->lastUsed = std::chrono::steady_clock::now();
	}
	s_numTextures--;
}

int Texture::GetNumInstances()
{
	return s_numTextures;
}

void Texture::LoadTexture(const std::string& path)
{
	LoadTextureOptimised(path, -1, -1);
}

void Texture::LoadTextureOptimised(const std::string& path, int width, int height)
{
	ImageHash hash = GetImageHash(path, width, height);

	auto it = textureCache.find(hash);
	if (it != textureCache.end())
	{
		m_cachedGpu = it->second;
		m_cachedGpu->inUse = true;
		m_cachedGpu->lastUsed = std::chrono::steady_clock::now();
		m_width = m_cachedGpu->width;
		m_height = m_cachedGpu->height;
		return;
	}

	auto tex = std::make_shared<CachedGpuTexture>();

	if (width > 0 && height > 0)
	{
		tex->width = width;
		tex->height = height;
	}
	else
	{
		tex->width = 1;
		tex->height = 1;
	}

	uint8_t placeholder[] = { 128, 128, 128 };
	tex->gpuTexture.SetData(placeholder, 1, 1, TextureFormat::RGB8);

	tex->state = TextureLoadState::Loading;
	tex->lastUsed = std::chrono::steady_clock::now();

	textureCache[hash] = tex;
	m_cachedGpu = tex;
	m_width = tex->width;
	m_height = tex->height;

	AsyncTextureLoader::GetInstance().QueueLoad(path, width, height, tex);
}

void Texture::RegisterDependancy(BaseComponent* component)
{
	if (!m_cachedGpu || !component)
		return;

	std::lock_guard<std::mutex> lock(m_cachedGpu->componentsMutex);

	auto it = std::find(m_cachedGpu->dependentComponents.begin(),
		m_cachedGpu->dependentComponents.end(),
		component);

	if (it == m_cachedGpu->dependentComponents.end())
	{
		m_cachedGpu->dependentComponents.push_back(component);
	}
}

void Texture::DeregisterDependancy(BaseComponent* component)
{
	if (!m_cachedGpu || !component)
		return;

	std::lock_guard<std::mutex> lock(m_cachedGpu->componentsMutex);

	m_cachedGpu->dependentComponents.erase(
		std::remove(m_cachedGpu->dependentComponents.begin(),
			m_cachedGpu->dependentComponents.end(),
			component),
		m_cachedGpu->dependentComponents.end()
	);
}

void Texture::GenerateDefaultTexture()
{
	uint32_t pixels[16] = {
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
	};

	auto tex = std::make_shared<CachedGpuTexture>();

	tex->gpuTexture.SetData((uint8_t*)pixels, 4, 4, TextureFormat::RGBA8);

	tex->width = 4;
	tex->height = 4;
	tex->channels = 4;
	tex->state = TextureLoadState::Uploaded;

	m_cachedGpu = tex;
}

void Texture::SetDefaultTexture(uint32_t texID)
{
	m_defaultTexID = texID;
}

void Texture::Bind()
{
	if (m_cachedGpu)
	{
		AsyncTextureLoader::GetInstance().ProcessCompletedLoads();

		m_cachedGpu->gpuTexture.Bind();
	}
}

bool Texture::IsLoaded() const
{
	return m_cachedGpu && m_cachedGpu->state == TextureLoadState::Uploaded;
}

TextureLoadState Texture::GetLoadState() const
{
	return m_cachedGpu ? m_cachedGpu->state : TextureLoadState::Empty;
}

void SableUI::StepCachedTexturesCleaner()
{
	static int iterations = 0;
	if (iterations++ < 500) return;
	iterations = 0;

	auto now = std::chrono::steady_clock::now();
	std::vector<ImageHash> toDelete;

	for (auto& [hash, tex] : textureCache)
	{
		if (tex.use_count() > 1) continue;
		if (!tex->inUse && now - tex->lastUsed > std::chrono::seconds(300))
			toDelete.push_back(hash);
	}

	for (auto& h : toDelete)
		textureCache.erase(h);

	AsyncTextureLoader::GetInstance().ProcessCompletedLoads();
}
