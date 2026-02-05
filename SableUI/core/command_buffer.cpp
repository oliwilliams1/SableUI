#include <SableUI/core/command_buffer.h>
#include <SableUI/core/renderer.h>
#include <cstring>
#include <cstdint>

using namespace SableUI;

void CommandBuffer::Reset()
{
	m_commands.clear();
	m_state = State{};
}

void CommandBuffer::SetPipeline(PipelineType pipeline)
{
	if (m_state.currentPipeline == pipeline)
		return;

	m_state.currentPipeline = pipeline;

	Command cmd;
	cmd.type = CommandType::SetPipeline;
	cmd.data = SetPipelineCmd{ pipeline };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::SetBlendState(bool enabled, BlendFactor src, BlendFactor dst)
{
	Command cmd;
	cmd.type = CommandType::SetBlendState;
	cmd.data = SetBlendStateCmd{ enabled, src, dst };
	m_commands.push_back(std::move(cmd));

	m_state.blendEnabled = enabled;
}

void CommandBuffer::SetScissor(int x, int y, int width, int height)
{
	Command cmd;
	cmd.type = CommandType::SetScissor;
	cmd.data = SetScissorCmd{ x, y, width, height };
	m_commands.push_back(std::move(cmd));

	m_state.scissorEnabled = true;
}

void CommandBuffer::DisableScissor()
{
	if (!m_state.scissorEnabled)
		return;

	Command cmd;
	cmd.type = CommandType::DisableScissor;
	m_commands.push_back(std::move(cmd));

	m_state.scissorEnabled = false;
}

void CommandBuffer::BindVertexBuffer(uint32_t vbo)
{
	Command cmd;
	cmd.type = CommandType::BindVertexBuffer;
	cmd.data = BindVertexBufferCmd{ vbo };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindIndexBuffer(uint32_t ebo)
{
	Command cmd;
	cmd.type = CommandType::BindIndexBuffer;
	cmd.data = BindIndexBufferCmd{ ebo };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindUniformBuffer(uint32_t binding, uint32_t ubo)
{
	Command cmd;
	cmd.type = CommandType::BindUniformBuffer;
	cmd.data = BindUniformBufferCmd{ binding, ubo };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindTexture(uint32_t slot, uint32_t texture)
{
	Command cmd;
	cmd.type = CommandType::BindTexture;
	cmd.data = BindTextureCmd{ slot, texture };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::UpdateUniformBuffer(uint32_t ubo, uint32_t offset, uint32_t size, const void* data)
{
	Command cmd;
	cmd.type = CommandType::UpdateUniformBuffer;
	cmd.data = UpdateUniformBufferCmd{ ubo, offset, size };

	// Copy data inline
	cmd.inlineData.resize(size);
	std::memcpy(cmd.inlineData.data(), data, size);

	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
	uint32_t firstIndex, int32_t vertexOffset,
	uint32_t firstInstance)
{
	Command cmd;
	cmd.type = CommandType::DrawIndexed;
	cmd.data = DrawIndexedCmd{ indexCount, instanceCount, firstIndex, vertexOffset, firstInstance };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount,
	uint32_t firstVertex, uint32_t firstInstance)
{
	Command cmd;
	cmd.type = CommandType::Draw;
	cmd.data = DrawCmd{ vertexCount, instanceCount, firstVertex, firstInstance };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::Clear(float r, float g, float b, float a)
{
	Command cmd;
	cmd.type = CommandType::Clear;
	cmd.data = ClearCmd{ r, g, b, a };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BeginRenderPass(const GpuFramebuffer* framebuffer)
{
	Command cmd;
	cmd.type = CommandType::BeginRenderPass;
	cmd.data = BeginRenderPassCmd{ framebuffer };
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::EndRenderPass()
{
	Command cmd;
	cmd.type = CommandType::EndRenderPass;
	m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BlitFramebuffer(uint32_t srcFBO, uint32_t dstFBO,
	int srcX0, int srcY0, int srcX1, int srcY1,
	int dstX0, int dstY0, int dstX1, int dstY1,
	TextureInterpolation filter)
{
	Command cmd;
	cmd.type = CommandType::BlitFramebuffer;
	cmd.data = BlitFramebufferCmd{ srcFBO, dstFBO, srcX0, srcY0, srcX1, srcY1,
								   dstX0, dstY0, dstX1, dstY1, filter };
	m_commands.push_back(std::move(cmd));
}