#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "SableUI/component.h"
#include "SableUI/memory.h"

namespace SableUI
{
    struct SplitterPanel;
    struct ContentPanel;

    struct BasePanel
    {
        BasePanel(BasePanel* parent, Renderer* renderer);
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
        virtual bool PropagateComponentStateChanges();

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
        SplitterPanel(BasePanel* parent, PanelType type, Renderer* renderer);
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
        DrawableSplitter m_drawable;
        bool m_drawableUpToDate = false;

        Colour m_bColour = { 51, 51, 51 };
    };

    struct ContentPanel : public BasePanel
    {
        ContentPanel(BasePanel* parent, Renderer* renderer);
        ~ContentPanel();
        static int GetNumInstances();

        void Render() override;
        SplitterPanel* AddSplitter(PanelType type) override;
        ContentPanel* AddPanel() override;

        void CalculateMinBounds() override;

        template<typename T, typename... Args>
        ContentPanel* AttachComponent(Args&&... args);
        void Update() override;

        void PropagateEvents(const UIEventContext& ctx) override;
        bool PropagateComponentStateChanges() override;

        BaseComponent* GetComponent() const { return m_component; }

    private:
        BaseComponent* m_component = nullptr;
    };

    template<typename T, typename... Args>
	inline ContentPanel* ContentPanel::AttachComponent(Args&&... args)
	{
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        if (m_component == nullptr) SableMemory::SB_delete(m_component);

        m_component = SableMemory::SB_new<T>(std::forward<Args>(args)...);
        m_component->BackendInitialisePanel(m_renderer);

        Update();
        return this;
	}
}