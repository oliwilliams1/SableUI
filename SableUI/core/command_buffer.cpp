#include <SableUI/renderer/renderer.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/utils/console.h>
#include <cstring>
#include <cstdint>
#include <utility>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "CommandBuffer"

using namespace SableUI;

void CommandBuffer::Reset()
{
    m_commands.clear();
    m_state = State{};
}

void CommandBuffer::SetPipeline(PipelineType pipeline)
{
    if (m_state.pipeline.has_value() && m_state.pipeline.value() == pipeline)
        return;

    Command cmd;
    cmd.type = CommandType::SetPipeline;
    cmd.data = SetPipelineCmd{ pipeline };
    m_commands.push_back(std::move(cmd));

    m_state.pipeline = pipeline;
}

void CommandBuffer::SetBlendState(bool enabled, BlendFactor src, BlendFactor dst)
{
    Command cmd;
    cmd.type = CommandType::SetBlendState;
    cmd.data = SetBlendStateCmd{ enabled, src, dst };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::SetScissor(int x, int y, int width, int height)
{
    Command cmd;
    cmd.type = CommandType::SetScissor;
    cmd.data = SetScissorCmd{ x, y, width, height };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DisableScissor()
{
    Command cmd;
    cmd.type = CommandType::DisableScissor;
    m_commands.push_back(std::move(cmd));
}

ResourceHandle CommandBuffer::CreateGpuObject(
    const void* vertices, uint32_t numVertices,
    const uint32_t* indices, uint32_t numIndices,
    const VertexLayout& layout)
{
    if (!m_allocator)
    {
        SableUI_Runtime_Error("CommandBuffer has no allocator - cannot create GPU object");
        return ResourceHandle{};
    }

    ResourceHandle handle = m_allocator->Allocate(ResourceType::GpuObject);

    if (!handle.IsValid())
    {
        SableUI_Runtime_Error("Failed to allocate resource handle for GPU object");
        return ResourceHandle{};
    }

    Command cmd;
    cmd.type = CommandType::CreateGpuObject;
    cmd.data = CreateGpuObjectCmd{
        handle,
        numVertices,
        numIndices,
        layout
    };

    size_t vertexDataSize = numVertices * layout.stride;
    size_t indexDataSize = numIndices * sizeof(uint32_t);
    size_t totalSize = vertexDataSize + indexDataSize;

    cmd.inlineData.resize(totalSize);
    std::memcpy(cmd.inlineData.data(), vertices, vertexDataSize);
    if (indices && numIndices > 0)
        std::memcpy(cmd.inlineData.data() + vertexDataSize, indices, indexDataSize);

    m_commands.push_back(std::move(cmd));

    return handle;
}

void SableUI::CommandBuffer::BindGpuObject(ResourceHandle handle)
{
    if (!handle.IsValid())
    {
        SableUI_Warn("Attempting to bind invalid GPU object handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BindGpuObject;
    cmd.data = BindGpuObjectCmd{ handle };
    m_commands.push_back(std::move(cmd));
}

void SableUI::CommandBuffer::DestroyGpuObject(ResourceHandle handle)
{
    if (!handle.IsValid())
    {
        SableUI_Warn("Attempting to destroy invalid GPU object handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::DestroyGpuObject;
    cmd.data = DestroyGpuObjectCmd{ handle };
    m_commands.push_back(std::move(cmd));

    if (m_allocator)
    {
        m_allocator->Free(handle);
    }
}

void CommandBuffer::BindUniformBuffer(uint32_t binding, uint32_t ubo)
{
    Command cmd;
    cmd.type = CommandType::BindUniformBuffer;
    cmd.data = BindUniformBufferCmd{ binding, ubo };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindTexture(uint32_t slot, const GpuTexture* texture)
{
    if (!texture)
        return;

    Command cmd;
    cmd.type = CommandType::BindTexture;
    cmd.data = BindTextureCmd{ slot, texture->GetHandle(), texture->GetType() };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::UpdateUniformBuffer(uint32_t ubo, uint32_t offset, uint32_t size, const void* data)
{
    Command cmd;
    cmd.type = CommandType::UpdateUniformBuffer;
    cmd.data = UpdateUniformBufferCmd{ ubo, offset, size };

    cmd.inlineData.resize(size);
    std::memcpy(cmd.inlineData.data(), data, size);

    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DrawGpuObject(ResourceHandle object, uint32_t instanceCount)
{
    Command cmd;
    cmd.type = CommandType::DrawGpuObject;
    cmd.data = DrawGpuObjectCmd{ object, instanceCount };
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