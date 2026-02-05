#include <glad/glad.h>
#include <SableUI/core/command_buffer.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/shader.h>
#include <SableUI/core/drawable.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <cstdint>
#include <variant>
#include <vector>

using namespace SableUI;

class OpenGLCommandExecutor : public CommandBufferExecutor
{
public:
	OpenGLCommandExecutor(GlobalResources* globalRes, ContextResources* contextRes)
		: m_globalRes(globalRes), m_contextRes(contextRes) {};

	void Execute(const CommandBuffer& cmdBuffer) override
	{
		for (const Command& cmd : cmdBuffer.GetCommands())
		{
			switch (cmd.type)
			{
			case CommandType::SetPipeline:
				ExecuteSetPipeline(std::get<SetPipelineCmd>(cmd.data));
				break;

			case CommandType::SetBlendState:
				ExecuteSetBlendState(std::get<SetBlendStateCmd>(cmd.data));
				break;

			case CommandType::SetScissor:
				ExecuteSetScissor(std::get<SetScissorCmd>(cmd.data));
				break;

			case CommandType::DisableScissor:
				glDisable(GL_SCISSOR_TEST);
				break;

			case CommandType::BindVertexBuffer:
				ExecuteBindVertexBuffer(std::get<BindVertexBufferCmd>(cmd.data));
				break;

			case CommandType::BindIndexBuffer:
				ExecuteBindIndexBuffer(std::get<BindIndexBufferCmd>(cmd.data));
				break;

			case CommandType::BindUniformBuffer:
				ExecuteBindUniformBuffer(std::get<BindUniformBufferCmd>(cmd.data));
				break;

			case CommandType::BindTexture:
				ExecuteBindTexture(std::get<BindTextureCmd>(cmd.data));
				break;

			case CommandType::UpdateUniformBuffer:
				ExecuteUpdateUniformBuffer(std::get<UpdateUniformBufferCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::DrawIndexed:
				ExecuteDrawIndexed(std::get<DrawIndexedCmd>(cmd.data));
				break;

			case CommandType::Draw:
				ExecuteDraw(std::get<DrawCmd>(cmd.data));
				break;

			case CommandType::Clear:
				ExecuteClear(std::get<ClearCmd>(cmd.data));
				break;

			case CommandType::BeginRenderPass:
				ExecuteBeginRenderPass(std::get<BeginRenderPassCmd>(cmd.data));
				break;

			case CommandType::EndRenderPass:
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				break;

			case CommandType::BlitFramebuffer:
				ExecuteBlitFramebuffer(std::get<BlitFramebufferCmd>(cmd.data));
				break;

			default:
				SableUI_Error("Unknown command type");
				break;
			}
		}
	}

private:
	GlobalResources* m_globalRes;
	ContextResources* m_contextRes;

	static GLenum BlendFactorToGL(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::Zero: return GL_ZERO;
		case BlendFactor::One: return GL_ONE;
		case BlendFactor::SrcColor: return GL_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor: return GL_DST_COLOR;
		case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha: return GL_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
		case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
		case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
		case BlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
		default: return GL_ONE;
		}
	}

	static GLenum TextureInterpolationToGL(TextureInterpolation interp)
	{
		switch (interp)
		{
		case TextureInterpolation::Nearest: return GL_NEAREST;
		case TextureInterpolation::Linear: return GL_LINEAR;
		default: return GL_NEAREST;
		}
	}

	void ExecuteSetPipeline(const SetPipelineCmd& cmd)
	{
		switch (cmd.pipeline)
		{
		case PipelineType::Rect:
			m_globalRes->s_rect.Use();
			break;
		case PipelineType::Text:
			m_globalRes->s_text.Use();
			break;
		case PipelineType::Image:
			m_globalRes->s_rect.Use();
			break;
		}
	}

	void ExecuteSetBlendState(const SetBlendStateCmd& cmd)
	{
		if (cmd.enabled)
		{
			glEnable(GL_BLEND);
			glBlendFunc(BlendFactorToGL(cmd.srcFactor), BlendFactorToGL(cmd.dstFactor));
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void ExecuteSetScissor(const SetScissorCmd& cmd)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(cmd.x, cmd.y, cmd.width, cmd.height);
	}

	void ExecuteBindVertexBuffer(const BindVertexBufferCmd& cmd)
	{
		glBindBuffer(GL_ARRAY_BUFFER, cmd.vbo);
	}

	void ExecuteBindIndexBuffer(const BindIndexBufferCmd& cmd)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.ebo);
	}

	void ExecuteBindUniformBuffer(const BindUniformBufferCmd& cmd)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, cmd.binding, cmd.ubo);
	}

	void ExecuteBindTexture(const BindTextureCmd& cmd)
	{
		glActiveTexture(GL_TEXTURE0 + cmd.slot);
		glBindTexture(GL_TEXTURE_2D, cmd.texture);
	}

	void ExecuteUpdateUniformBuffer(const UpdateUniformBufferCmd& cmd, const std::vector<uint8_t>& data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, cmd.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, cmd.offset, cmd.size, data.data());
	}

	void ExecuteDrawIndexed(const DrawIndexedCmd& cmd)
	{
		if (cmd.instanceCount > 1)
		{
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				cmd.indexCount,
				GL_UNSIGNED_INT,
				(void*)(cmd.firstIndex * sizeof(uint32_t)),
				cmd.instanceCount,
				cmd.vertexOffset
			);
		}
		else
		{
			glDrawElements(
				GL_TRIANGLES,
				cmd.indexCount,
				GL_UNSIGNED_INT,
				(void*)(cmd.firstIndex * sizeof(uint32_t))
			);
		}
	}

	void ExecuteDraw(const DrawCmd& cmd)
	{
		if (cmd.instanceCount > 1)
		{
			glDrawArraysInstanced(GL_TRIANGLES, cmd.firstVertex, cmd.vertexCount, cmd.instanceCount);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, cmd.firstVertex, cmd.vertexCount);
		}
	}

	void ExecuteClear(const ClearCmd& cmd)
	{
		glClearColor(cmd.r, cmd.g, cmd.b, cmd.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void ExecuteBeginRenderPass(const BeginRenderPassCmd& cmd)
	{
		if (!cmd.framebuffer->isWindowSurface)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, cmd.framebuffer->GetHandle());
			glViewport(0, 0, cmd.framebuffer->GetColorAttachments()[0].GetWidth(),
				cmd.framebuffer->GetColorAttachments()[0].GetHeight());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void ExecuteBlitFramebuffer(const BlitFramebufferCmd& cmd)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, cmd.srcFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cmd.dstFBO);
		glBlitFramebuffer(
			cmd.srcX0, cmd.srcY0, cmd.srcX1, cmd.srcY1,
			cmd.dstX0, cmd.dstY0, cmd.dstX1, cmd.dstY1,
			GL_COLOR_BUFFER_BIT,
			TextureInterpolationToGL(cmd.filter)
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

SableUI::CommandBufferExecutor* SableUI::CommandBufferExecutor::Create(Backend backend, GlobalResources* globalRes, ContextResources* contextRes)
{ 
	switch (backend)
	{
	case SableUI::Backend::OpenGL:
		return SableMemory::SB_new<OpenGLCommandExecutor>(globalRes, contextRes);
		break;
	default:
		SableUI_Error("Resorting to OpenGL command buffer executor");
		return SableMemory::SB_new<OpenGLCommandExecutor>(globalRes, contextRes);
		break;
	}
}