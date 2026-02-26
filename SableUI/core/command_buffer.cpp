#include <SableUI/renderer/command_buffer.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/utils/console.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <utility>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "CommandBuffer"

using namespace SableUI;

void CommandBuffer::Reset()
{
    m_commands.clear();
    m_state = State{};
}

// ============================================================================
// Pipeline State
// ============================================================================

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

void CommandBuffer::SetViewport(int x, int y, int width, int height)
{
    Command cmd;
    cmd.type = CommandType::SetViewport;
    cmd.data = SetViewportCmd{ x, y, width, height };
    m_commands.push_back(std::move(cmd));
}

// ============================================================================
// Resource Binding
// ============================================================================

void CommandBuffer::BindUniformBuffer(uint32_t binding, ResourceHandle buffer)
{
    if (!buffer.IsValid())
    {
        SableUI_Warn("Attempting to bind invalid uniform buffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BindUniformBuffer;
    cmd.data = BindUniformBufferCmd{ binding, buffer };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindTexture(uint32_t slot, ResourceHandle texture)
{
    if (!texture.IsValid())
    {
        SableUI_Warn("Attempting to bind invalid texture handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BindTexture;
    cmd.data = BindTextureCmd{ slot, texture };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BindGpuObject(ResourceHandle handle)
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

void CommandBuffer::BindFramebuffer(ResourceHandle framebuffer)
{
    if (!framebuffer.IsValid())
    {
        SableUI_Warn("Attempting to bind invalid framebuffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BindFramebuffer;
    cmd.data = BindFramebufferCmd{ framebuffer };
    m_commands.push_back(std::move(cmd));
}

// ============================================================================
// GPU Object Management
// ============================================================================

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

void CommandBuffer::DestroyGpuObject(ResourceHandle handle)
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
        m_allocator->Free(handle);
}

// ============================================================================
// Texture Management
// ============================================================================

ResourceHandle CommandBuffer::CreateTexture2D(
    int width, int height,
    TextureFormat format,
    TextureUsage usage)
{
    if (!m_allocator)
    {
        SableUI_Runtime_Error("CommandBuffer has no allocator - cannot create texture");
        return ResourceHandle{};
    }

    ResourceHandle handle = m_allocator->Allocate(ResourceType::Texture2D);

    if (!handle.IsValid())
    {
        SableUI_Runtime_Error("Failed to allocate resource handle for Texture2D");
        return ResourceHandle{};
    }

    Command cmd;
    cmd.type = CommandType::CreateTexture2D;
    cmd.data = CreateTexture2DCmd{ handle, width, height, format, usage };
    m_commands.push_back(std::move(cmd));

    return handle;
}

void CommandBuffer::SetDataTexture2D(ResourceHandle texture,
    const uint8_t* pixels, int width, int height,
    TextureFormat format)
{
    if (!texture.IsValid())
    {
        SableUI_Warn("Attempting to set data on invalid texture handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::SetDataTexture2D;
    cmd.data = SetTextureDataCmd{ texture, width, height, format };

    int channels = 4;
    switch (format)
    {
    case TextureFormat::RGB8: channels = 3; break;
    case TextureFormat::RG8: channels = 2; break;
    case TextureFormat::R8: channels = 1; break;
    default: break;
    }

    size_t dataSize = static_cast<size_t>(width) * height * channels;
    cmd.inlineData.resize(dataSize);
    std::memcpy(cmd.inlineData.data(), pixels, dataSize);

    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::CreateStorageTexture2D(ResourceHandle texture,
    int width, int height,
    TextureFormat format,
    TextureUsage usage)
{
    if (!texture.IsValid())
    {
        SableUI_Warn("Attempting to create storage for invalid texture handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::CreateStorageTexture2D;
    cmd.data = CreateTextureStorageCmd{ texture, width, height, format, usage };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DestroyTexture(ResourceHandle handle)
{
    if (!handle.IsValid())
    {
        SableUI_Warn("Attempting to destroy invalid Texture2D handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::DestroyTexture;
    cmd.data = DestroyTextureCmd{ handle };
    m_commands.push_back(std::move(cmd));

    if (m_allocator)
        m_allocator->Free(handle);
}

ResourceHandle CommandBuffer::CreateTexture2DArray(
    int width, int height, int depth,
    TextureFormat format,
    TextureUsage usage)
{
    if (!m_allocator)
    {
        SableUI_Runtime_Error("CommandBuffer has no allocator - cannot create texture");
        return ResourceHandle{};
    }

    ResourceHandle handle = m_allocator->Allocate(ResourceType::Texture2DArray);

    if (!handle.IsValid())
    {
        SableUI_Runtime_Error("Failed to allocate resource handle for Texture2DArray");
        return ResourceHandle{};
    }

    Command cmd;
    cmd.type = CommandType::CreateTexture2DArray;
    cmd.data = CreateTexture2DArrayCmd{ handle, width, height, depth, format, usage };
    m_commands.push_back(std::move(cmd));

    return handle;
}

void SableUI::CommandBuffer::SubImageTexture2DArray(ResourceHandle texture,
    int xOffset, int yOffset, int zOffset,
    int width, int height, int depth,
    TextureFormat format,
    const uint8_t* pixels)
{
    if (!texture.IsValid())
    {
        SableUI_Warn("Attempting to set data on invalid texture handle");
        return;
    }

    int channels = 4;
    switch (format)
    {
    case TextureFormat::RGB8: channels = 3; break;
    case TextureFormat::RG8: channels = 2; break;
    case TextureFormat::R8: channels = 1; break;
    default: break;
    }

    Command cmd;
    cmd.type = CommandType::SubImageTexture2DArray;
    cmd.data = SubImageTexture2DArrayCmd{ texture, xOffset, yOffset, zOffset, width, height, depth, format };

    size_t dataSize = static_cast<size_t>(width) * height * depth * channels;
    cmd.inlineData.resize(dataSize);
    std::memcpy(cmd.inlineData.data(), pixels, dataSize);

    m_commands.push_back(std::move(cmd));
}

void SableUI::CommandBuffer::CopyImageDataTexture2DArray(ResourceHandle src, ResourceHandle dst,
    int srcX, int srcY, int srcZ,
    int dstX, int dstY, int dstZ,
    int width, int height, int depth)
{
	if (!src.IsValid() || !dst.IsValid())
	{
		SableUI_Warn("Attempting to set data on invalid texture handle");
		return;
	}

    Command cmd;
    cmd.type = CommandType::CopyImageDataTexture2DArray;
    cmd.data = CopyImageDataTexture2DArrayCmd{ src, dst, srcX, srcY, srcZ, dstX, dstY, dstZ, width, height, depth };
    m_commands.push_back(std::move(cmd));
}

void SableUI::CommandBuffer::ResizeTexture2DArray(ResourceHandle handle, int newDepth)
{
	if (!handle.IsValid())
	{
		SableUI_Warn("Attempting to resize invalid Texture2DArray handle");
		return;
	}

    Command cmd;
    cmd.type = CommandType::ResizeTexture2DArray;
    cmd.data = ResizeTexture2DArrayCmd{ handle, newDepth };
	m_commands.push_back(std::move(cmd));
}

// ============================================================================
// Framebuffer Management
// ============================================================================

ResourceHandle CommandBuffer::CreateFramebuffer(
    int width, int height,
    bool isWindowSurface)
{
    if (!m_allocator)
    {
        SableUI_Runtime_Error("CommandBuffer has no allocator - cannot create framebuffer");
        return ResourceHandle{};
    }

    ResourceHandle handle = m_allocator->Allocate(ResourceType::Framebuffer);

    if (!handle.IsValid())
    {
        SableUI_Runtime_Error("Failed to allocate resource handle for Framebuffer");
        return ResourceHandle{};
    }

    Command cmd;
    cmd.type = CommandType::CreateFramebuffer;
    cmd.data = CreateFramebufferCmd{ handle, width, height, isWindowSurface };
    m_commands.push_back(std::move(cmd));

    return handle;
}

void CommandBuffer::AttachColourTexture(ResourceHandle framebuffer,
    ResourceHandle texture, int slot)
{
    if (!framebuffer.IsValid() || !texture.IsValid())
    {
        SableUI_Warn("Attempting to attach texture with invalid handle(s)");
        return;
    }

    Command cmd;
    cmd.type = CommandType::AttachColourTexture;
    cmd.data = AttachColorTextureCmd{ framebuffer, texture, slot };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::AttachDepthStencilTexture(ResourceHandle framebuffer,
    ResourceHandle texture)
{
    if (!framebuffer.IsValid() || !texture.IsValid())
    {
        SableUI_Warn("Attempting to attach depth/stencil texture with invalid handle(s)");
        return;
    }

    Command cmd;
    cmd.type = CommandType::AttachDepthStencilTexture;
    cmd.data = AttachDepthStencilTextureCmd{ framebuffer, texture };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BakeFramebuffer(ResourceHandle framebuffer)
{
    if (!framebuffer.IsValid())
    {
        SableUI_Warn("Attempting to bake invalid framebuffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BakeFramebuffer;
    cmd.data = BakeFramebufferCmd{ framebuffer };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::SetFramebufferSize(ResourceHandle framebuffer,
    int width, int height)
{
    if (!framebuffer.IsValid())
    {
        SableUI_Warn("Attempting to resize invalid framebuffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::SetFramebufferSize;
    cmd.data = SetFramebufferSizeCmd{ framebuffer, width, height };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DestroyFramebuffer(ResourceHandle handle)
{
    if (!handle.IsValid())
    {
        SableUI_Warn("Attempting to destroy invalid framebuffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::DestroyFramebuffer;
    cmd.data = DestroyFramebufferCmd{ handle };
    m_commands.push_back(std::move(cmd));

    if (m_allocator)
        m_allocator->Free(handle);
}

// ============================================================================
// Uniform Buffer Management
// ============================================================================

ResourceHandle CommandBuffer::CreateUniformBuffer(uint32_t size, const void* initialData)
{
    if (!m_allocator)
    {
        SableUI_Runtime_Error("CommandBuffer has no allocator - cannot create uniform buffer");
        return ResourceHandle{};
    }

    ResourceHandle handle = m_allocator->Allocate(ResourceType::UniformBuffer);

    if (!handle.IsValid())
    {
        SableUI_Runtime_Error("Failed to allocate resource handle for UniformBuffer");
        return ResourceHandle{};
    }

    Command cmd;
    cmd.type = CommandType::CreateUniformBuffer;
    cmd.data = CreateUniformBufferCmd{ handle, size };

    if (initialData)
    {
        cmd.inlineData.resize(size);
        std::memcpy(cmd.inlineData.data(), initialData, size);
    }

    m_commands.push_back(std::move(cmd));

    return handle;
}

void CommandBuffer::UpdateUniformBuffer(ResourceHandle buffer, uint32_t offset, uint32_t size, const void* data)
{
    if (!buffer.IsValid())
    {
        SableUI_Warn("Attempting to update invalid uniform buffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::UpdateUniformBuffer;
    cmd.data = UpdateUniformBufferCmd{ buffer, offset, size };

    cmd.inlineData.resize(size);
    std::memcpy(cmd.inlineData.data(), data, size);

    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::DestroyUniformBuffer(ResourceHandle handle)
{
    if (!handle.IsValid())
    {
        SableUI_Warn("Attempting to destroy invalid uniform buffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::DestroyUniformBuffer;
    cmd.data = DestroyUniformBufferCmd{ handle };
    m_commands.push_back(std::move(cmd));

    if (m_allocator)
        m_allocator->Free(handle);
}

// ============================================================================
// Drawing
// ============================================================================

void CommandBuffer::DrawGpuObject(ResourceHandle object, uint32_t instanceCount)
{
    if (!object.IsValid())
    {
        SableUI_Warn("Attempting to draw invalid GPU object handle");
        return;
    }

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

// ============================================================================
// Render Pass
// ============================================================================

void CommandBuffer::Clear(float r, float g, float b, float a)
{
    Command cmd;
    cmd.type = CommandType::Clear;
    cmd.data = ClearCmd{ r, g, b, a };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BeginRenderPass(ResourceHandle framebuffer)
{
    if (!framebuffer.IsValid())
    {
        SableUI_Warn("Attempting to begin render pass with invalid framebuffer handle");
        return;
    }

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

// ============================================================================
// Framebuffer Operations
// ============================================================================

void CommandBuffer::BlitFramebuffer(ResourceHandle srcFramebuffer, ResourceHandle dstFramebuffer,
    int srcX0, int srcY0, int srcX1, int srcY1,
    int dstX0, int dstY0, int dstX1, int dstY1,
    TextureInterpolation filter)
{
    if (!srcFramebuffer.IsValid() || !dstFramebuffer.IsValid())
    {
        SableUI_Warn("Attempting to blit with invalid framebuffer handle(s)");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BlitFramebuffer;
    cmd.data = BlitFramebufferCmd{
        srcFramebuffer, dstFramebuffer,
        srcX0, srcY0, srcX1, srcY1,
        dstX0, dstY0, dstX1, dstY1,
        filter
    };
    m_commands.push_back(std::move(cmd));
}

void CommandBuffer::BlitToScreen(ResourceHandle framebuffer, TextureInterpolation filter)
{
    if (!framebuffer.IsValid())
    {
        SableUI_Warn("Attempting to blit to screen with invalid framebuffer handle");
        return;
    }

    Command cmd;
    cmd.type = CommandType::BlitToScreen;
    cmd.data = BlitToScreenCmd{ framebuffer, filter };
    m_commands.push_back(std::move(cmd));
}

static const char* CommandTypeToString(CommandType type)
{
    switch (type)
    {
    case CommandType::SetPipeline:                return "SetPipeline";
    case CommandType::SetBlendState:              return "SetBlendState";
    case CommandType::SetScissor:                 return "SetScissor";
    case CommandType::DisableScissor:             return "DisableScissor";
    case CommandType::Clear:                      return "Clear";
    case CommandType::BlitFramebuffer:            return "BlitFramebuffer";
    case CommandType::BlitToScreen:               return "BlitToScreen";
    case CommandType::SetViewport:                return "SetViewport";
    case CommandType::CreateGpuObject:            return "CreateGpuObject";
    case CommandType::BindGpuObject:              return "BindGpuObject";
    case CommandType::DrawGpuObject:              return "DrawGpuObject";
    case CommandType::DestroyGpuObject:           return "DestroyGpuObject";
    case CommandType::CreateTexture2D:            return "CreateTexture2D";
    case CommandType::CreateStorageTexture2D:     return "CreateStorageTexture2D";
    case CommandType::BindTexture:                return "BindTexture";
    case CommandType::DestroyTexture:             return "DestroyTexture";
    case CommandType::SetDataTexture2D:           return "SetDataTexture2D";
    case CommandType::CreateTexture2DArray:       return "CreateTexture2DArray";
    case CommandType::ResizeTexture2DArray:       return "ResizeTexture2DArray";
    case CommandType::SubImageTexture2DArray:     return "SubImageTexture2DArray";
    case CommandType::CopyImageDataTexture2DArray:return "CopyImageDataTexture2DArray";
    case CommandType::CreateUniformBuffer:        return "CreateUniformBuffer";
    case CommandType::BindUniformBuffer:          return "BindUniformBuffer";
    case CommandType::UpdateUniformBuffer:        return "UpdateUniformBuffer";
    case CommandType::DestroyUniformBuffer:       return "DestroyUniformBuffer";
    case CommandType::CreateFramebuffer:          return "CreateFramebuffer";
    case CommandType::DestroyFramebuffer:         return "DestroyFramebuffer";
    case CommandType::BindFramebuffer:            return "BindFramebuffer";
    case CommandType::AttachColourTexture:        return "AttachColourTexture";
    case CommandType::AttachDepthStencilTexture:  return "AttachDepthStencilTexture";
    case CommandType::BakeFramebuffer:            return "BakeFramebuffer";
    case CommandType::SetFramebufferSize:         return "SetFramebufferSize";
    case CommandType::DrawIndexed:                return "DrawIndexed";
    case CommandType::Draw:                       return "Draw";
    case CommandType::BeginRenderPass:            return "BeginRenderPass";
    case CommandType::EndRenderPass:              return "EndRenderPass";
    default:                                      return "Unknown";
    }
}

void CommandBuffer::DebugPrintAndClear() const
{
    //if (m_commands.size() < 10) return;
    //printf("\033[2J\033[H");
    //printf("=== CommandBuffer (%zu commands)\n", m_commands.size());

    //for (size_t i = 0; i < m_commands.size(); i++)
    //    printf("  [%03zu] %s\n", i, CommandTypeToString(m_commands[i].type));

    //printf("\n\n");
    //fflush(stdout);
}
