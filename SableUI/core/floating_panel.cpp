#include <SableUI/core/floating_panel.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/renderer.h>
#include <SableUI/utils/utils.h>

using namespace SableMemory;

static int s_panelCount = 0;

SableUI::FloatingPanel::FloatingPanel(RendererBackend* renderer, const Rect& p_rect)
    : BasePanel(nullptr, renderer)
{
    s_panelCount++;
    type = PanelType::Floating;
    rect = p_rect;

    m_colourAttachment.CreateStorage(rect.w, rect.h, TextureFormat::RGBA8, TextureUsage::RenderTarget);
    m_framebuffer.SetSize(rect.w, rect.h);
    m_framebuffer.AttachColour(&m_colourAttachment, 0);
    m_framebuffer.Bake();
}

SableUI::FloatingPanel::~FloatingPanel()
{
    SB_delete(m_component);
    s_panelCount--;
}

int SableUI::FloatingPanel::GetNumInstances()
{
    return s_panelCount;
}

SableUI::SplitterPanel* SableUI::FloatingPanel::AddSplitter(PanelType type)
{
    SableUI_Error("Floating panel cannot have splitter children");
    return nullptr;
}

SableUI::ContentPanel* SableUI::FloatingPanel::AddPanel()
{
    SableUI_Error("Floating panel cannot have panel children");
    return nullptr;
}

void SableUI::FloatingPanel::Update()
{
    if (m_component == nullptr)
    {
        m_component = SB_new<BaseComponent>();
        m_component->SetRenderer(m_renderer);
        m_component->BackendInitialisePanel();
    }

    Rect localRect = { 0, 0, rect.w, rect.h };
    m_component->GetRootElement()->SetRect(localRect);
    m_component->GetRootElement()->LayoutChildren();

    m_needsRender = true;
}

void SableUI::FloatingPanel::Render()
{
    if (!m_component || !m_needsRender)
        return;

    m_renderer->BeginRenderPass(&m_framebuffer);
    m_renderer->Clear(0.0f, 0.0f, 0.0f, 0.0f);
    m_component->Render();
    m_renderer->Draw(&m_framebuffer);
    m_renderer->EndRenderPass();

    m_needsRender = false;
}

void SableUI::FloatingPanel::DistributeEvents(const UIEventContext& ctx)
{
    if (!m_component)
        return;

    UIEventContext localCtx = ctx;
    localCtx.mousePos.x -= rect.x;
    localCtx.mousePos.y -= rect.y;

    m_component->HandleInput(localCtx);
}

bool SableUI::FloatingPanel::UpdateComponents()
{
    if (!m_component)
        return false;

    bool changed = m_component->CheckAndUpdate();

    if (changed)
    {
        m_component->GetRootElement()->LayoutChildren();
        Update();
    }

    return changed;
}

void SableUI::FloatingPanel::PostLayoutUpdate(const UIEventContext& ctx)
{
    if (!m_component)
        return;

    UIEventContext localCtx = ctx;
    localCtx.mousePos.x -= rect.x;
    localCtx.mousePos.y -= rect.y;

    m_component->PostLayoutUpdate(localCtx);
}

SableUI::Element* SableUI::FloatingPanel::GetElementById(const SableString& id)
{
    if (!m_component)
        return nullptr;

    return m_component->GetElementById(id);
}

void SableUI::FloatingPanel::Resize(int w, int h)
{
    if (rect.w == w && rect.h == h)
        return;

    rect.w = w;
    rect.h = h;

    m_colourAttachment.CreateStorage(w, h, TextureFormat::RGBA8, TextureUsage::RenderTarget);
    m_framebuffer.SetSize(w, h);

    Update();
}

const SableUI::GpuFramebuffer* SableUI::FloatingPanel::GetFramebuffer() const
{
    return &m_framebuffer;
}