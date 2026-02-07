#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace SableUI
{
	void StepCachedTexturesCleaner();

	struct CachedGpuTexture;

	enum class TextureLoadState
	{
		Empty,
		Loading,
		Loaded,
		Uploaded,
		Failed
	};

	struct TextureLoadRequest
	{
		std::string path;
		int targetWidth;
		int targetHeight;
		std::shared_ptr<CachedGpuTexture> target;
	};

	class AsyncTextureLoader
	{
	public:
		static AsyncTextureLoader& GetInstance();

		void Initialise();
		void Shutdown();

		void QueueLoad(const std::string& path, int width, int height, std::shared_ptr<CachedGpuTexture> target);

		void ProcessCompletedLoads();

	private:
		AsyncTextureLoader() = default;
		~AsyncTextureLoader();

		void WorkerThread();

		std::thread m_worker;
		std::atomic<bool> m_running{ false };

		std::mutex m_queueMutex;
		std::condition_variable m_queueCV;
		std::queue<TextureLoadRequest> m_loadQueue;

		std::mutex m_completedMutex;
		std::queue<TextureLoadRequest> m_completedQueue;
	};

	class BaseComponent;
	struct GpuTexture2D;
	struct Texture
	{
		Texture();
		Texture(int width, int height, uint32_t texID);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		static int GetNumInstances();

		void LoadTexture(const std::string& path);
		void LoadTextureOptimised(const std::string& path, int width = -1, int height = -1);

		void SetDefaultTexture(uint32_t texID);
		void Bind();

		bool IsLoaded() const;
		TextureLoadState GetLoadState() const;

		void RegisterDependancy(BaseComponent* component);
		void DeregisterDependancy(BaseComponent* component);

		int m_width = -1;
		int m_height = -1;

		const GpuTexture2D* GetGpuTexture() const;

	private:
		void GenerateDefaultTexture();

		uint32_t m_defaultTexID = 0;
		std::shared_ptr<CachedGpuTexture> m_cachedGpu;
	};
}