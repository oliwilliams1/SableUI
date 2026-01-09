#pragma once
#include <SableUI/core/panel.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/renderer.h>
#include <SableUI/utils/utils.h>
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