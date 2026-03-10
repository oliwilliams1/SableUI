#pragma once
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/core/events.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/drawable.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/element.h>
#include <vector>
#include <string>

namespace SableUI
{
    struct SplitterPanel;
    struct ContentPanel;

    struct BasePanel
    {
        BasePanel(BasePanel* parent, RendererBackend* renderer);
        virtual ~BasePanel();
        static int GetNumInstances();

        virtual void Render(const DrawableDrawData& drData) {};
        virtual void Recalculate(const DrawableDrawData& drData) {};

        virtual SplitterPanel* AddSplitter(CommandBuffer& cmd, PanelType type) = 0;
        virtual ContentPanel* AddPanel(CommandBuffer& cmd) = 0;

        virtual void CalculateScales() {};
        virtual void CalculatePositions(const DrawableDrawData& drData) {};
        virtual void CalculateMinBounds() {};
        virtual void Update(const DrawableDrawData& drData) {};

        virtual void DistributeEvents(const UIEventContext& ctx);
        virtual bool UpdateComponents(const DrawableDrawData& drData);
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
        void Render(const DrawableDrawData& drData) override;
        void Recalculate(const DrawableDrawData& drData) override;

        SplitterPanel* AddSplitter(CommandBuffer& cmd, PanelType type) override;
        ContentPanel* AddPanel(CommandBuffer& cmd) override;

        void CalculateScales() override;
        void CalculatePositions(const DrawableDrawData& drData) override;
    };

    struct SplitterPanel : public BasePanel
    {
        SplitterPanel(BasePanel* parent, PanelType type, RendererBackend* renderer);
        ~SplitterPanel();
        static int GetNumInstances();

        void Render(const DrawableDrawData& drData) override;

        SplitterPanel* AddSplitter(CommandBuffer& cmd, PanelType type) override;
        ContentPanel* AddPanel(CommandBuffer& cmd) override;

        void CalculateScales() override;
        void CalculatePositions(const DrawableDrawData& drData) override;
        void CalculateMinBounds() override;
        void Update(const DrawableDrawData& drData) override;

        int bSize = 1;

    private:
        DrawableSplitter* m_drawable;
        bool m_drawableUpToDate = false;
    };

    class BaseComponent;
    struct ContentPanel : public BasePanel
    {
        ContentPanel(BasePanel* parent, RendererBackend* renderer);
        ~ContentPanel();
        static int GetNumInstances();

        void Render(const DrawableDrawData& drData) override;
        SplitterPanel* AddSplitter(CommandBuffer& cmd, PanelType type) override;
        ContentPanel* AddPanel(CommandBuffer& cmd) override;
        BaseComponent* AttachComponent(const std::string& componentName);

        void Update(const DrawableDrawData& drData) override;

        void DistributeEvents(const UIEventContext& ctx) override;
        bool UpdateComponents(const DrawableDrawData& drData) override;
        void PostLayoutUpdate(const UIEventContext& ctx) override;

        BaseComponent* GetComponent() const { return m_component; }
        Element* GetElementById(const SableString& id) override;

    private:
        BaseComponent* m_component = nullptr;
    };
}