#pragma once

#include <vector>
#include <type_traits>

#include <SableUI/component.h>
#include <SableUI/memory.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h>

namespace SableUI
{
    struct SplitterPanel;
    struct ContentPanel;

    struct BasePanel
    {
        BasePanel(BasePanel* parent, RendererBackend* renderer);
        virtual ~BasePanel();
        static int GetNumInstances();

        virtual void Render() = 0;
        virtual void Recalculate() {};

        virtual SplitterPanel* AddSplitter(PanelType type) = 0;
        virtual ContentPanel* AddPanel() = 0;

        virtual void CalculateScales() {};
        virtual void CalculatePositions() {};
        virtual void CalculateMinBounds() {};
        virtual void Update() {};

        virtual void PropagateEvents(const UIEventContext& ctx);
        virtual void PropagatePostLayoutEvents(const UIEventContext& ctx);
        virtual bool PropagateComponentStateChanges();
        virtual Element* GetElementById(const SableString& id);

        BasePanel* parent = nullptr;
        SableUI::Rect rect = { 0, 0, 0, 0 };
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

        Colour m_bColour = { 51, 51, 51 };
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

        void PropagateEvents(const UIEventContext& ctx) override;
        void PropagatePostLayoutEvents(const UIEventContext& ctx) override;
        bool PropagateComponentStateChanges() override;

        BaseComponent* GetComponent() const { return m_component; }
        Element* GetElementById(const SableString& id) override;

    private:
        BaseComponent* m_component = nullptr;
    };
}