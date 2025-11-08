#pragma once
#include "SableUI/drawable.h"

namespace SableUI
{
	// RenderTarget
	enum class RenderTargetType
	{
		Window,
		Texture
	};

	struct RenderTarget
	{
		RenderTarget() = default;
		RenderTarget(int width, int height);
		~RenderTarget();

		void SetTarget(RenderTargetType target) { this->targetType = target; };
		void InitTexture();
		void Resize(int width, int height);

		void Bind() const;

		int width = 0, height = 0;
		RenderTargetType targetType = RenderTargetType::Window;

	private:
		uint32_t m_textureID = 0;
		void Update() const;
	};

	// Renderer
	enum class Backend { UNDEF, OpenGL, Vulkan };

	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate
	};

	struct RenderTarget;
	class sRenderer
	{
	public:
		~sRenderer() = default;
		void InitOpenGL();

		void Clear(float r, float g, float b, float a);
		void Viewport(int x, int y, int width, int height);
		void SetBlending(bool enabled);
		void SetBlendFunction(BlendFactor src, BlendFactor dst);
		void Flush();

		void CheckErrors();

	public:
		void ClearDrawableStack();
		void ClearDrawable(DrawableBase* drawable);

		void Draw(DrawableBase* drawable);
		void Draw();

		void StartDirectDraw();
		void DirectDrawRect(const Rect& rect, const Colour& color);
		void EndDirectDraw();

		RenderTarget m_renderTarget;
	
	private:
		Backend m_backend = Backend::UNDEF;

	private:
		bool m_directDraw = false;
		std::vector<DrawableBase*> m_drawStack;
	};

	struct GpuTexture
	{
	public:
		~GpuTexture();
		void Bind() const;
		void Unbind() const;
		void SetData(uint8_t* pixels, int width, int height, int channels);
		bool ResizeDepth(int newDepth);

	private:
		int m_width = 0, m_height = 0, m_depth = 0;
		uint32_t m_textureID = 0;
	};
}