#pragma once
#include <SableUI/core/window.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/renderer.h>
#include <SableUI/styles/styles.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component_registry.h>
#include <SableUI/styles/theme.h>
#include <SableUI/core/scroll_context.h>
#include <type_traits>
#include <utility>

/* non-macro user api */
namespace SableUI
{
	struct ElementInfo;
	class Element;

	void PreInit(int argc, char** argv);
	void SetBackend(const Backend& backend);

	Window* Initialise(const char* name = "SableUI", int width = 800, int height = 600, const WindowInitInfo& info = {});
	Window* CreateSecondaryWindow(const char* name = "Unnamed window", int width = 800, int height = 600, const WindowInitInfo& info = {});
	void Shutdown();

	void SetContext(Window* window);
	Window* GetContext();

	SableString GetClipboardContent();
	void SetClipboardContent(const SableString& str);

	bool PollEvents();
	bool WaitEvents();
	bool WaitEventsTimeout(double timeout);

	void Render();

	void PostEmptyEvent();

	void SetElementBuilderContext(RendererBackend* renderer, Element* rootElement, bool isVirtual);
	void SetCurrentComponent(BaseComponent* component);
	Element* GetCurrentElement();
	VirtualNode* GetVirtualRootNode();

	SplitterPanel* StartSplitter(PanelType orientation);
	void EndSplitter();
	ContentPanel* AddPanel();

	void StartDivVirtual(const ElementInfo& info = {}, BaseComponent* child = nullptr);
	void EndDivVirtual();
	void AddRectVirtual(const ElementInfo& info = {});
	void AddImageVirtual(const SableString& path, const ElementInfo& info = {});
	void AddTextVirtual(const SableString& text, const ElementInfo& info = {});

	void StartDiv(const ElementInfo& info = {}, SableUI::BaseComponent* child = nullptr);
	void EndDiv();
	void AddRect(const ElementInfo& info = {});
	void AddImage(const SableString& path, const ElementInfo& info = {});
	void AddText(const SableString& text, const ElementInfo& info = {});

	void SetNextPanelMaxWidth(int width);
	void SetNextPanelMaxHeight(int height);
	void SetNextPanelMinBounds(ivec2 bounds);

	void StartCustomLayoutScope(CustomTargetQueue* queuePtr);
	void EndCustomLayoutScope(CustomTargetQueue* queuePtr);

	void CreateFloatingPanel(const std::string& id, const std::string& componentName, const Rect& r = { 0, 0, 100, 100 });
	void QueueDestroyFloatingPanel(const std::string& id);

	struct DivScope
	{
	public:
		explicit DivScope(const SableUI::ElementInfo& info)
		{
			SableUI::StartDiv(info);
		}
		~DivScope()
		{
			SableUI::EndDiv();
		}
		DivScope(const DivScope&) = delete;
		DivScope& operator=(const DivScope&) = delete;
		DivScope(DivScope&&) = default;
		DivScope& operator=(DivScope&&) = default;
	};

	struct SplitterScope
	{
	public:
		explicit SplitterScope(const PanelType& type)
		{
			SableUI::StartSplitter(type);
		}
		~SplitterScope()
		{
			SableUI::EndSplitter();
		}
		SplitterScope(const SplitterScope&) = delete;
		SplitterScope& operator=(const SplitterScope&) = delete;
		SplitterScope(SplitterScope&&) = default;
		SplitterScope& operator=(SplitterScope&&) = default;
	};

	template<typename... Args>
	inline ElementInfo PackStyles(Args&&... args) {
		ElementInfo info;
		(args.ApplyTo(info), ...);
		return info;
	}

	template<typename... Args>
	inline void PackStylesToInfo(ElementInfo& info, Args&&... args) {
		(args.ApplyTo(info), ...);
	}

	inline ElementInfo StripAppearanceStyles(ElementInfo info) {
		info.appearance = AppearanceProps{};
		return info;
	}
}

/* scoped RAII rect guard api */
#define Div(...) if (SableUI::DivScope CONCAT(_d, __LINE__)						\
					(SableUI::PackStyles(__VA_ARGS__)); true)

#define RectElement(...) AddRect(SableUI::PackStyles(__VA_ARGS__))
#define Image(path, ...) AddImage(path, SableUI::PackStyles(__VA_ARGS__))
#define Text(text, ...) AddText(text, SableUI::PackStyles(__VA_ARGS__))

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define STRINGIFY(x) #x

#define Component(name, ...) AddComponent(name)									\
	->BackendInitialiseChild(name, this, SableUI::PackStyles(__VA_ARGS__))

#define ComponentScoped(T, name, owner, ...)									\
	if (auto CONCAT(_cs_, __LINE__) =											\
			ComponentScope<T>(													\
				owner,															\
				STRINGIFY(T),													\
				SableUI::PackStyles(__VA_ARGS__)								\
			);																	\
		T* name = CONCAT(_cs_, __LINE__).get())

#define ComponentScopedWithStyle(T, name, owner, style)							\
	if (auto CONCAT(_cs_, __LINE__) =											\
			ComponentScope<T>(													\
				owner,															\
				STRINGIFY(T),													\
				style															\
			);																	\
		T* name = CONCAT(_cs_, __LINE__).get())

// Horizontal splitter
#define HSplitter()	if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)	\
						(SableUI::PanelType::HorizontalSplitter); true)
// Vertical splitter
#define VSplitter()	if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)	\
						(SableUI::PanelType::VerticalSplitter); true)

#define EmptyPanel()	SableUI::AddPanel()
#define Panel(name)		SableUI::AddPanel()->AttachComponent(name)->BackendInitialisePanel()

#define PanelGainRefNoInit(name, T, ref)										\
	T* ref = nullptr;															\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__)							\
		= SableUI::AddPanel()->AttachComponent(name);							\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)					\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));						\
	else																		\
		SableUI_Runtime_Error(													\
			"Component '%s' does not match requried type: %s",					\
			name, STRINGIFY(T))

#define PanelGainRef(name, T, ref)												\
	PanelGainRefNoInit(name, T, ref);											\
	CONCAT(_comp_, __LINE__)->BackendInitialisePanel()

#define PanelGainRefWithInit(name, T, ref, initLines)							\
	PanelGainRefNoInit(name, T, ref);											\
	initLines;																	\
	CONCAT(_comp_, __LINE__)->BackendInitialisePanel();

#include <SableUI/components/button.h>
#include <SableUI/components/checkbox.h>
#include <SableUI/components/text_field.h>
#include <SableUI/core/tab_context.h>

// Visual horizontal splitter element
#define SplitterHorizontal(...)													\
	RectElement(mx(2), my(4), h(1), w_fill, bg(70, 70, 70) __VA_ARGS__)

// Visual vertical splitter element
#define SplitterVertical(...)													\
	RectElement(my(4), mx(2), h_fill, w(1), bg(70, 70, 70) __VA_ARGS__)

// Text with splitters either side
#define TextSeperator(label)													\
	Div(left_right, h_fit, w_fill, centerY, mt(8))								\
	{																			\
		RectElement(mx(2), h(1), w(6), bg(70, 70, 70), centerY);				\
		Text(label, w_fit, wrapText(false), mx(2), mb(4));						\
		RectElement(mx(2), h(1), w_fill, bg(70, 70, 70), centerY);				\
	}
	