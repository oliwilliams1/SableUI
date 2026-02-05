#pragma once
#include <SableUI/core/shader.h>
#include <cstdint>
#include <vector>
#include <variant>
#include <cstdint>

namespace SableUI
{
	class CommandBuffer;
	struct GpuObject;
	struct GpuFramebuffer;
	enum class BlendFactor;
	enum class TextureInterpolation;

	enum class Backend { Undef, OpenGL, Vulkan, DirectX, Metal };

	struct GlobalResources {
		Shader s_rect;
		uint32_t ubo_rect = 0;

		Shader s_text;
		uint32_t ubo_text = 0;
		uint32_t u_textAtlas = 0;
	};

	struct ContextResources {
		GpuObject* rectObject = nullptr;
	};

	enum class CommandType : uint8_t
	{
		SetPipeline,
		SetBlendState,
		SetScissor,
		DisableScissor,
		BindVertexBuffer,
		BindIndexBuffer,
		BindUniformBuffer,
		BindTexture,
		BindGpuObject,
		UpdateUniformBuffer,
		DrawIndexed,
		Draw,
		Clear,
		BeginRenderPass,
		EndRenderPass,
		BlitFramebuffer,
	};

	enum class PipelineType : uint8_t
	{
		Rect,
		Text,
		Image,
	};

	struct SetPipelineCmd
	{
		PipelineType pipeline;
	};

	struct SetBlendStateCmd
	{
		bool enabled;
		BlendFactor srcFactor;
		BlendFactor dstFactor;
	};

	struct SetScissorCmd
	{
		int x, y, width, height;
	};

	struct BindVertexBufferCmd
	{
		uint32_t vbo;
	};

	struct BindIndexBufferCmd
	{
		uint32_t ebo;
	};

	struct BindUniformBufferCmd
	{
		uint32_t binding;
		uint32_t ubo;
	};

	struct BindTextureCmd
	{
		uint32_t slot;
		uint32_t texture;
	};

	struct UpdateUniformBufferCmd
	{
		uint32_t ubo;
		uint32_t offset;
		uint32_t size;
	};

	struct DrawIndexedCmd
	{
		uint32_t indexCount;
		uint32_t instanceCount;
		uint32_t firstIndex;
		int32_t vertexOffset;
		uint32_t firstInstance;
	};

	struct DrawCmd
	{
		uint32_t vertexCount;
		uint32_t instanceCount;
		uint32_t firstVertex;
		uint32_t firstInstance;
	};

	struct ClearCmd
	{
		float r, g, b, a;
	};

	struct BeginRenderPassCmd
	{
		const GpuFramebuffer* framebuffer;
	};

	struct BlitFramebufferCmd
	{
		uint32_t srcFBO;
		uint32_t dstFBO;
		int srcX0, srcY0, srcX1, srcY1;
		int dstX0, dstY0, dstX1, dstY1;
		TextureInterpolation filter;
	};

	using CommandData = std::variant<
		SetPipelineCmd,
		SetBlendStateCmd,
		SetScissorCmd,
		BindVertexBufferCmd,
		BindIndexBufferCmd,
		BindUniformBufferCmd,
		BindTextureCmd,
		UpdateUniformBufferCmd,
		DrawIndexedCmd,
		DrawCmd,
		ClearCmd,
		BeginRenderPassCmd,
		BlitFramebufferCmd
	>;

	struct Command
	{
		CommandType type;
		CommandData data;
		std::vector<uint8_t> inlineData;
	};

	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		~CommandBuffer() = default;

		void Reset();

		void SetPipeline(PipelineType pipeline);
		void SetBlendState(bool enabled, BlendFactor src, BlendFactor dst);
		void SetScissor(int x, int y, int width, int height);
		void DisableScissor();

		void BindVertexBuffer(uint32_t vbo);
		void BindIndexBuffer(uint32_t ebo);
		void BindUniformBuffer(uint32_t binding, uint32_t ubo);
		void BindTexture(uint32_t slot, uint32_t texture);

		void UpdateUniformBuffer(uint32_t ubo, uint32_t offset, uint32_t size, const void* data);

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
			uint32_t firstIndex = 0, int32_t vertexOffset = 0,
			uint32_t firstInstance = 0);
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
			uint32_t firstVertex = 0, uint32_t firstInstance = 0);

		void Clear(float r, float g, float b, float a);

		void BeginRenderPass(const GpuFramebuffer* framebuffer);
		void EndRenderPass();

		void BlitFramebuffer(uint32_t srcFBO, uint32_t dstFBO,
			int srcX0, int srcY0, int srcX1, int srcY1,
			int dstX0, int dstY0, int dstX1, int dstY1,
			TextureInterpolation filter);

		const std::vector<Command>& GetCommands() const { return m_commands; }
		bool empty() const { return m_commands.empty(); }
		size_t GetCommandCount() const { return m_commands.size(); }

	private:
		std::vector<Command> m_commands;

		struct State
		{
			PipelineType currentPipeline = PipelineType::Rect;
			bool blendEnabled = false;
			bool scissorEnabled = false;
		} m_state;
	};

	class CommandBufferExecutor
	{
	public:
		static CommandBufferExecutor* Create(Backend backend, GlobalResources* globalRes, ContextResources* contextRes);
		virtual ~CommandBufferExecutor() = default;
		virtual void Execute(const CommandBuffer& cmdBuffer) = 0;
	};
}