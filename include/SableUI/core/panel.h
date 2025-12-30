#pragma once

#include <vector>
#include <type_traits>

#include <SableUI/core/component.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/events.h>
#include <SableUI/core/renderer.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>

namespace SableUI
{
    struct SplitterPanel;
    struct ContentPanel;

    struct BasePanel
    {
        BasePanel(BasePanel* parent, RendererBackend* renderer);
        virtual ~BasePanel();
        static int GetNumInstances();

        virtual void Render() {};
        virtual void Recalculate() {};

        virtual SplitterPanel* AddSplitter(PanelType type) = 0;
        virtual ContentPanel* AddPanel() = 0;

        virtual void CalculateScales() {};
        virtual void CalculatePositions() {};
        virtual void CalculateMinBounds() {};
        virtual void Update() {};

        virtual void DistributeEvents(const UIEventContext& ctx);
        virtual bool UpdateComponents();
        virtual void PostLayoutUpdate(const UIEventContext& ctx);

        virtual Element* GetElementById(const SableString& id);

        BasePanel* parent = nullptr;
        SableUI::Rect rect = { 0, 0, 0, 0 };
        RectType wType = RectType::Undef;
        RectType hType = RectType::Undef;
        ivec2 minBounds = { 20, 20 };
        ivec2 maxBounds = { 0, 0 };
        PanelType type = PanelType::Undef;
        std::vector<BasePanel*> children;

    protected:
        ivec2 mousePos = { 0, 0 };
        RendererBackend* m_renderer = nullptr;
        SableUI::BasePanel* FindRoot();

        bool isFocused = true;
    };

    struct RootPanel : public BasePanel
    {
        RootPanel(RendererBackend* renderer, int w, int h);
        ~RootPanel();
        static int GetNumInstances();

        void Resize(int w, int h);
        void Render() override;
        void Recalculate() override;

        SplitterPanel* AddSplitter(PanelType type) override;
        ContentPanel* AddPanel() override;

        void CalculateScales() override;
        void CalculatePositions() override;
    };

    struct SplitterPanel : public BasePanel
    {
        SplitterPanel(BasePanel* parent, PanelType type, RendererBackend* renderer);
        ~SplitterPanel();
        static int GetNumInstances();

        void Render() override;

        SplitterPanel* AddSplitter(PanelType type) override;
        ContentPanel* AddPanel() override;

        void CalculateScales() override;
        void CalculatePositions() override;
        void CalculateMinBounds() override;
        void Update() override;

        int bSize = 1;

    private:
        DrawableSplitter* m_drawable;
        bool m_drawableUpToDate = false;
    };

    struct ContentPanel : public BasePanel
    {
        ContentPanel(BasePanel* parent, RendererBackend* renderer);
        ~ContentPanel();
        static int GetNumInstances();

        void Render() override;
        SplitterPanel* AddSplitter(PanelType type) override;
        ContentPanel* AddPanel() override;
        BaseComponent* AttachComponent(const std::string& componentName);

        void Update() override;

        void DistributeEvents(const UIEventContext& ctx) override;
        bool UpdateComponents() override;
        void PostLayoutUpdate(const UIEventContext& ctx) override;

        BaseComponent* GetComponent() const { return m_component; }
        Element* GetElementById(const SableString& id) override;

    private:
        BaseComponent* m_component = nullptr;
    };
}