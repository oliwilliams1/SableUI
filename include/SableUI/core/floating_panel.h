#pragma once
#include <SableUI/core/panel.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/renderer.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <type_traits>
#include <string>

namespace SableUI
{
    struct FloatingPanel : public BasePanel
    {
    public:
        FloatingPanel(RendererBackend* renderer, const Rect& rect);
        ~FloatingPanel();
        static int GetNumInstances();

        void Render() override;
        void Recalculate() override {};

        SplitterPanel* AddSplitter(PanelType type) override;
        ContentPanel* AddPanel() override;
        BaseComponent* AttachComponent(const std::string& componentName);

        template <typename T>
        T* AttachComponentByType();

        void Update() override;
        void Resize(int w, int h);

        void DistributeEvents(const UIEventContext& ctx) override;
        bool UpdateComponents() override;
        void PostLayoutUpdate(const UIEventContext& ctx) override;

        BaseComponent* GetComponent() const { return m_component; }
        Element* GetElementById(const SableString& id) override;

        const GpuFramebuffer* GetFramebuffer() const;
        bool IsDirty() const { return m_needsRender || (m_renderer && m_renderer->isDirty()); }

        void ClearDrawStack() { m_renderer->ClearDrawableStack(); }
        RendererBackend* GetRenderer() { return m_renderer; }

    private:
        BaseComponent* m_component = nullptr;
        GpuFramebuffer m_framebuffer;
        GpuTexture2D m_colourAttachment;
        bool m_needsRender = true;
    };

    template <typename T>
    inline T* FloatingPanel::AttachComponentByType()
    {
        static_assert(std::is_base_of_v<BaseComponent, T>,
            "FloatingPanel::AttachComponentByType<T>: T must derive from BaseComponent");

        if (m_component != nullptr)
            SableMemory::SB_delete(m_component);

        T* comp = SableMemory::SB_new<T>();
        comp->SetRenderer(m_renderer);

        return comp;
    }
}