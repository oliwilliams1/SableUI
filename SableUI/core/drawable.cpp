#include <SableUI/core/shader.h>
#include <SableUI/generated/shaders.h>
#include <SableUI/core/drawable.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/window.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/utils.h>
#include <algorithm>
#include <vector>
#include <map>
#include <optional>
#include <cstdint>

using namespace SableUI;

// rect globals
static std::map<void*, SableUI::ContextResources> g_contextResources;
static GlobalResources g_res{};

struct Vertex {
	SableUI::vec2 uv;
};

Vertex rectVertices[] = {
	{{ 0.0f, 0.0f }},
	{{ 1.0f, 0.0f }},
	{{ 1.0f, 1.0f }},
	{{ 0.0f, 1.0f }}
};

unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

static inline void RectToNDC(
	const Rect& r,
	const Rect& fbo,
	float& x, float& y, float& w, float& h)
{
	x = ((r.x - fbo.x) / (float)fbo.w);
	y = ((r.y - fbo.y) / (float)fbo.h);
	w = (r.w / (float)fbo.w);
	h = (r.h / (float)fbo.h);

	x = x * 2.0f - 1.0f;
	y = y * 2.0f - 1.0f;
	w *= 2.0f;
	h *= 2.0f;

	w = (std::max)(0.0f, w);
	h = (std::max)(0.0f, h);

	y *= -1.0f;
	h *= -1.0f;
}

// ============================================================================
// Global Resources
// ============================================================================
void SableUI::DestroyGlobalResources(CommandBuffer& cmd, RendererBackend* renderer)
{
	if (!g_res.initialised)
		return;

	if (g_res.ubo_rect.IsValid())
		cmd.DestroyUniformBuffer(g_res.ubo_rect);

	if (g_res.ubo_text.IsValid())
		cmd.DestroyUniformBuffer(g_res.ubo_text);

	renderer->ExecuteCommandBuffer(cmd);
	cmd.Reset();

	g_res.initialised = false;
}

GlobalResources& SableUI::GetGlobalResources()
{
	return g_res;
}

// ============================================================================
// Context Resources
// ============================================================================
void SableUI::SetupContextResources(CommandBuffer& cb, RendererBackend* renderer)
{
	if (!g_res.initialised)
	{
		g_res.s_rect.LoadBasicShaders(rect_vert, rect_frag);
		g_res.s_text.LoadBasicShaders(text_vert, text_frag);

		g_res.ubo_rect = cb.CreateUniformBuffer(sizeof(RectDrawData), nullptr);
		g_res.ubo_text = cb.CreateUniformBuffer(sizeof(TextDrawData), nullptr);

		g_res.initialised = true;
	}

	void* ctx = GetCurrentContext_voidType();
	ContextResources& resources = g_contextResources[ctx];

	VertexLayout layout;
	layout.Add(VertexFormat::Float2);

	ResourceHandle handle = cb.CreateGpuObject(
		rectVertices,
		sizeof(rectVertices) / sizeof(Vertex),
		indices,
		sizeof(indices) / sizeof(unsigned int),
		layout
	);

	if (!handle.IsValid())
	{
		SableUI_Runtime_Error("Failed to create rect GPU object");
	}

	resources.rectObject = handle;

	cb.BindUniformBuffer(static_cast<uint32_t>(UboBinding::Rect), g_res.ubo_rect);
	cb.BindUniformBuffer(static_cast<uint32_t>(UboBinding::Text), g_res.ubo_text);
}

void SableUI::DestroyContextResources(CommandBuffer& cmd, RendererBackend* renderer)
{
	void* ctx = SableUI::GetCurrentContext_voidType();

	auto it = g_contextResources.find(ctx);
	if (it != g_contextResources.end())
	{
		auto& res = it->second;

		if (res.rectObject.IsValid())
			cmd.DestroyGpuObject(res.rectObject);

		renderer->ExecuteCommandBuffer(cmd);
	}

	g_contextResources.clear();
}

ContextResources& SableUI::GetContextResources(RendererBackend* renderer)
{
	void* ctx = GetCurrentContext_voidType();

	auto it = g_contextResources.find(ctx);
	if (it != g_contextResources.end())
	{
		return it->second;
	}

	SableUI_Runtime_Error("GetContextResources failed to find resources");
}

// ============================================================================
// DrawableBase
// ============================================================================
static int s_drawableBaseCount = 0;

DrawableBase::DrawableBase()
{
	s_drawableBaseCount++;
	this->uuid = GetUUID();
}

DrawableBase::~DrawableBase()
{
	s_drawableBaseCount--;
}

int DrawableBase::GetNumInstances()
{
	return s_drawableBaseCount;
}

unsigned int DrawableBase::GetUUID()
{
	static unsigned int s_nextUUID = 0;
	return s_nextUUID++;
}

// ============================================================================
// DrawableRect
// ============================================================================
static int s_drawableRectCount = 0;

DrawableRect::DrawableRect()
{
	s_drawableRectCount++;
	this->m_zIndex = 0;
}

DrawableRect::~DrawableRect()
{
	s_drawableRectCount--;
}

int DrawableRect::GetNumInstances()
{
	return s_drawableRectCount;
}

void DrawableRect::Update(
	const Rect& rect,
	std::optional<Colour> colour,
	float rTL, float rTR,
	float rBL, float rBR,
	std::optional<Colour> borderColour,
	int bT, int bB,
	int bL, int bR,
	bool clipEnabled,
	const Rect& clipRect)
{
	this->m_rect = rect;
	this->m_colour = colour;
	this->m_rTL = rTL;
	this->m_rTR = rTR;
	this->m_rBL = rBL;
	this->m_rBR = rBR;
	this->m_borderColour = borderColour;
	this->m_bT = bT;
	this->m_bB = bB;
	this->m_bL = bL;
	this->m_bR = bR;
	this->scissorEnabled = clipEnabled;
	this->scissorRect = clipRect;
}

void DrawableRect::RecordCommands(const DrawableDrawData& drData)
{
	float x, y, w, h;
	RectToNDC(m_rect, drData.fbRect, x, y, w, h);

	RectDrawData data{};

	data.rect[0] = x;
	data.rect[1] = y;
	data.rect[2] = w;
	data.rect[3] = h;

	Colour c = m_colour.value_or(Colour{ 0, 0, 0, 0 });
	data.colour[0] = c.r / 255.0f;
	data.colour[1] = c.g / 255.0f;
	data.colour[2] = c.b / 255.0f;
	data.colour[3] = c.a / 255.0f;

	Colour bc = m_borderColour.value_or(Colour{ 0, 0, 0, 0 });
	data.borderColour[0] = bc.r / 255.0f;
	data.borderColour[1] = bc.g / 255.0f;
	data.borderColour[2] = bc.b / 255.0f;
	data.borderColour[3] = bc.a / 255.0f;

	data.realRect[0] = m_rect.x - drData.fbRect.x;
	data.realRect[1] = m_rect.y - drData.fbRect.y;
	data.realRect[2] = m_rect.w;
	data.realRect[3] = m_rect.h;

	data.radius[0] = m_rTL;
	data.radius[1] = m_rTR;
	data.radius[2] = m_rBL;
	data.radius[3] = m_rBR;

	data.borderSize[0] = m_bT;
	data.borderSize[1] = m_bB;
	data.borderSize[2] = m_bL;
	data.borderSize[3] = m_bR;

	data.useTexture = 0;

	drData.cmd.SetPipeline(PipelineType::Rect);
	drData.cmd.UpdateUniformBuffer(g_res.ubo_rect, 0, sizeof(RectDrawData), &data);
	drData.cmd.DrawGpuObject(drData.contextResources.rectObject);
}

// ============================================================================
// DrawableSplitter
// ============================================================================
static int s_drawableSplitterCount = 0;

DrawableSplitter::DrawableSplitter()
{
	s_drawableSplitterCount++;
	this->m_zIndex = 1;
}

DrawableSplitter::DrawableSplitter(Rect& r, Colour colour)
	: m_colour(colour)
{
	s_drawableSplitterCount++;
	this->m_zIndex = 999;
	this->m_rect = r;
}

DrawableSplitter::~DrawableSplitter()
{
	s_drawableSplitterCount--;
	m_offsets.clear();
}

int DrawableSplitter::GetNumInstances()
{
	return s_drawableSplitterCount;
}

void DrawableSplitter::Update(Rect& rect, Colour colour, PanelType type,
	float pBSize, const std::vector<int>& segments)
{
	this->m_rect = rect;
	this->m_colour = colour;
	this->m_type = type;
	this->m_bSize = SableUI::f2i(pBSize);
	this->m_offsets = segments;
}

void DrawableSplitter::RecordCommands(const DrawableDrawData& drData)
{
	if (m_type == PanelType::Undef ||
		m_type == PanelType::Base ||
		m_type == PanelType::Root)
		return;

	drData.cmd.SetPipeline(PipelineType::Rect);

	RectDrawData data{};
	Colour c = m_colour;

	data.colour[0] = c.r / 255.0f;
	data.colour[1] = c.g / 255.0f;
	data.colour[2] = c.b / 255.0f;
	data.colour[3] = c.a / 255.0f;

	data.useTexture = 0;

	int startX = std::clamp(m_rect.x, 0, drData.fbRect.w);
	int startY = std::clamp(m_rect.y, 0, drData.fbRect.h);
	int boundW = std::clamp(m_rect.w, 0, drData.fbRect.w- startX);
	int boundH = std::clamp(m_rect.h, 0, drData.fbRect.h - startY);

	auto drawRect = [&](Rect r) {
		float x, y, w, h;
		RectToNDC(r, drData.fbRect, x, y, w, h);

		data.rect[0] = x;
		data.rect[1] = y;
		data.rect[2] = w;
		data.rect[3] = h;

		data.realRect[0] = m_rect.x - drData.fbRect.x;
		data.realRect[1] = m_rect.y - drData.fbRect.y;
		data.realRect[2] = r.w;
		data.realRect[3] = r.h;

		drData.cmd.UpdateUniformBuffer(g_res.ubo_rect, 0, sizeof(RectDrawData), &data);
		drData.cmd.DrawGpuObject(drData.contextResources.rectObject);
	};

	if (m_type == PanelType::HorizontalSplitter)
	{
		for (int offset : m_offsets)
		{
			drawRect({
				startX + offset - m_bSize,
				startY,
				m_bSize * 2,
				boundH
			});
		}
	}
	else
	{
		for (int offset : m_offsets)
		{
			drawRect({
				startX,
				startY + offset - m_bSize,
				boundW,
				m_bSize * 2
			});
		}
	}
}

// ============================================================================
// DrawableImage
// ============================================================================
static int s_drawableImageCount = 0;

DrawableImage::DrawableImage()
{
	s_drawableImageCount++;
	this->m_zIndex = 0;
}

DrawableImage::~DrawableImage()
{
	s_drawableImageCount--;
}

int DrawableImage::GetNumInstances()
{
	return s_drawableImageCount;
}

void SableUI::DrawableImage::Update(
	Rect& rect,
	float rTL, float rTR,
	float rBL, float rBR,
	std::optional<Colour> borderColour,
	int bT, int bB,
	int bL, int bR,
	bool clipEnabled,
	const Rect& clipRect)
{
	this->m_rect = rect;
	this->m_rTL = rTL;
	this->m_rTR = rTR;
	this->m_rBL = rBL;
	this->m_rBR = rBR;
	this->m_borderColour = borderColour;
	this->m_bT = bT;
	this->m_bB = bB;
	this->m_bL = bL;
	this->m_bR = bR;
	this->scissorEnabled = clipEnabled;
	this->scissorRect = clipRect;
}

void DrawableImage::RecordCommands(const DrawableDrawData& drData)
{
	RectDrawData data{};

	float x, y, w, h;
	RectToNDC(m_rect, drData.fbRect, x, y, w, h);

	data.rect[0] = x;
	data.rect[1] = y;
	data.rect[2] = w;
	data.rect[3] = h;
	
	data.realRect[0] = m_rect.x - drData.fbRect.x;
	data.realRect[1] = m_rect.y - drData.fbRect.y;
	data.realRect[2] = m_rect.w;
	data.realRect[3] = m_rect.h;

	data.radius[0] = m_rTL;
	data.radius[1] = m_rTR;
	data.radius[2] = m_rBL;
	data.radius[3] = m_rBR;

	data.borderSize[0] = m_bT;
	data.borderSize[1] = m_bB;
	data.borderSize[2] = m_bL;
	data.borderSize[3] = m_bR;

	if (m_borderColour)
	{
		Colour bc = *m_borderColour;
		data.borderColour[0] = bc.r / 255.0f; data.borderColour[1] = bc.g / 255.0f;
		data.borderColour[2] = bc.b / 255.0f; data.borderColour[3] = bc.a / 255.0f;
	}

	data.useTexture = 1;

	drData.cmd.SetPipeline(PipelineType::Image);

	ResourceHandle texHandle = m_texture.GetGpuTexture();
	if (texHandle.IsValid())
		drData.cmd.BindTexture(0, texHandle);

	drData.cmd.UpdateUniformBuffer(g_res.ubo_rect, 0, sizeof(RectDrawData), &data);
	drData.cmd.DrawGpuObject(drData.contextResources.rectObject);
}

void SableUI::DrawableImage::RegisterTextureDependancy(BaseComponent* component)
{
	m_texture.RegisterDependancy(component);
}

void SableUI::DrawableImage::DeregisterTextureDependancy(BaseComponent* component)
{
	m_texture.DeregisterDependancy(component);
}

// ============================================================================
// DrawableText
// ============================================================================
static int s_drawableTextCount = 0;

DrawableText::DrawableText()
{
	s_drawableTextCount++;
	this->m_zIndex = 0;
}

DrawableText::~DrawableText()
{
	s_drawableTextCount--;
}

int DrawableText::GetNumInstances()
{
	return s_drawableTextCount;
}

void SableUI::DrawableText::Update(Rect& rect, bool clipEnabled, const Rect& clipRect)
{
	this->m_rect = rect;
	this->scissorEnabled = clipEnabled;
	this->scissorRect = clipRect;
};

void DrawableText::RecordCommands(const DrawableDrawData& drData)
{
	if (!m_text.m_gpuObject.IsValid())
		return;

	TextDrawData data{};
	data.targetSize[0] = static_cast<float>(drData.fbRect.w);
	data.targetSize[1] = static_cast<float>(drData.fbRect.h);
	data.pos[0] = static_cast<float>(m_rect.x - drData.fbRect.x);
	data.pos[1] = static_cast<float>((m_rect.y + m_rect.h) - drData.fbRect.y);

	drData.cmd.SetPipeline(PipelineType::Text);

	ResourceHandle atlasTexture = SableUI::GetTextAtlasTexture();
	if (atlasTexture.IsValid())
		drData.cmd.BindTexture(0, atlasTexture);

	drData.cmd.UpdateUniformBuffer(g_res.ubo_text, 0, sizeof(TextDrawData), &data);
	drData.cmd.DrawGpuObject(m_text.m_gpuObject);
}