#pragma once
#include <string>
#include <vector>
#include <memory>
#include <SableUI/utils/utils.h>

namespace SableUI
{
	class BaseComponent;
	class RendererBackend;
	struct UIEventContext;

	struct FloatingPanelInfo
	{
		std::string id;
		Rect bounds{ 100, 100, 400, 300 };
		int zIndex = 100;
		bool modal = false;
	};

	class FloatingPanel
	{
	public:
		FloatingPanel(const std::string& id, RendererBackend* renderer, const FloatingPanelInfo& info);
		~FloatingPanel();

		void AttachComponent(BaseComponent* component);
		void Update();
		void Render();
		void HandleInput(const UIEventContext& ctx, bool& handled);

		bool IsPointInside(ivec2 point) const;
		void SetBounds(const Rect& bounds);
		void SetZIndex(int zIndex) { m_zIndex = zIndex; }

		const std::string& GetID() const { return m_id; }
		BaseComponent* GetComponent() const { return m_component; }
		Rect GetBounds() const { return m_bounds; }
		int GetZIndex() const { return m_zIndex; }
		bool IsModal() const { return m_modal; }

	private:
		std::string m_id;
		BaseComponent* m_component = nullptr;
		RendererBackend* m_renderer = nullptr;
		Rect m_bounds;
		int m_zIndex;
		bool m_modal;
	};

	class FloatingPanelManager
	{
	public:
		FloatingPanelManager(RendererBackend* renderer);
		~FloatingPanelManager();

		FloatingPanel* OpenPanel(const std::string& componentName, const FloatingPanelInfo& info);
		void ClosePanel(const std::string& id);
		bool IsPanelActive(const std::string& id) const;
		FloatingPanel* GetPanel(const std::string& id) const;

		void UpdateAll();
		void RenderAll();
		void HandleInput(const UIEventContext& ctx, bool& blockMainContent);

		void BringToFront(const std::string& id);

	private:
		RendererBackend* m_renderer;
		std::vector<FloatingPanel*> m_panels;

		void SortByZIndex();
	};

	namespace FloatingPanels
	{
		FloatingPanel* Open(const std::string& componentName, const FloatingPanelInfo& info);
		void Close(const std::string& id);
		bool IsActive(const std::string& id);
		FloatingPanel* Get(const std::string& id);
		void BringToFront(const std::string& id);
	}
}