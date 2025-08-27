#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "SableUI/component.h"

namespace SableUI
{
    struct SplitterPanel;
    struct Panel;

    struct BasePanel
    {
        BasePanel(BasePanel* parent, Renderer* renderer);
        virtual ~BasePanel() = default;
        virtual void Render() = 0;
        virtual void Recalculate() {};

        virtual SplitterPanel* AddSplitter(PanelType type) = 0;
        virtual Panel* AddPanel() = 0;

        virtual void CalculateScales() {};
        virtual void CalculatePositions() {};
        virtual void CalculateMinBounds() {};
        virtual void Update() {};

        virtual void HandleHoverEventPanel(const ivec2& mousePos);
        virtual void PropagateComponentStateChanges();

        BasePanel* parent = nullptr;
        SableUI::Rect rect = { 0, 0, 0, 0 };
        ivec2 minBounds = { 20, 20 };
        PanelType type = PanelType::UNDEF;
        std::vector<BasePanel*> children;

    protected:
        ivec2 mousePos = { 0, 0 };
        Renderer* m_renderer = nullptr;
        SableUI::BasePanel* FindRoot();

        bool isFocused = true;
    };

    struct RootPanel : public BasePanel
    {
        RootPanel(Renderer* renderer, int w, int h);
        ~RootPanel();

        void Resize(int w, int h);
        void Render() override;
        void Recalculate() override;

        SplitterPanel* AddSplitter(PanelType type) override;
        Panel* AddPanel() override;

        void CalculateScales() override;
        void CalculatePositions() override;
    };

    struct SplitterPanel : public BasePanel
    {
        SplitterPanel(BasePanel* parent, PanelType type, Renderer* renderer);
        ~SplitterPanel();

        void Render() override;

        SplitterPanel* AddSplitter(PanelType type) override;
        Panel* AddPanel() override;

        void CalculateScales() override;
        void CalculatePositions() override;
        void CalculateMinBounds() override;
        void Update() override;

        int bSize = 1;
    private:
        DrawableSplitter m_drawable;
        bool m_drawableUpToDate = false;

        Colour m_bColour = { 51, 51, 51 };
    };

    struct Panel : public BasePanel
    {
        Panel(BasePanel* parent, Renderer* renderer);

        void Render() override;
        SplitterPanel* AddSplitter(PanelType type) override;
        Panel* AddPanel() override;

        template<typename T, typename... Args>
        Panel* AttachComponent(Args&&... args)
        {
            static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

            if (m_component == nullptr) delete m_component;

            m_component = new T(std::forward<Args>(args)...);
            m_component->BackendInitialise(m_renderer);

            Update();
            return this;
        }

        void Update() override;

        void HandleHoverEventPanel(const ivec2& mousePos) override;
        void PropagateComponentStateChanges() override;

    private:
        BaseComponent* m_component = nullptr;
    };
}