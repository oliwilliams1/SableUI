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
#include <SableUI/core/tab_context.h>
#include <SableUI/components/menu_bar.h>

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

	void StartCustomLayoutScope(CustomTargetQueue* queuePtr, const ElementInfo& ElementInfo = {});
	void EndCustomLayoutScope(CustomTargetQueue* queuePtr);

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
		info.text = TextProps{};
		return info;
	}
}

/* scoped RAII rect guard api */
#define Rect(...) AddRect(SableUI::PackStyles(__VA_ARGS__))
#define Div(...) if (SableUI::DivScope CONCAT(_d, __LINE__)(SableUI::PackStyles(__VA_ARGS__)); true)
#define Image(path, ...) AddImage(path, SableUI::PackStyles(__VA_ARGS__))
#define Text(text, ...) AddText(text, SableUI::PackStyles(__VA_ARGS__))

#define STRINGIFY(x) #x

#define Component(name, ...) AddComponent(name)																\
	->BackendInitialiseChild(name, this, SableUI::PackStyles(__VA_ARGS__))

#define ComponentGainBaseRef(name, ref, ...)																\
	SableUI::BaseComponent* ref = AddComponent(name);														\
	ref->BackendInitialiseChild(name, this, SableUI::PackStyles(__VA_ARGS__))

#define ComponentGainRef(name, T, ref, ...)																	\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = AddComponent(name);									\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match requried type: %s", name, STRINGIFY(T));		\
	ref->BackendInitialiseChild(name, this, SableUI::PackStyles(__VA_ARGS__))

#define ComponentGainRefNoInit(name, T, ref)																\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = AddComponent(name);									\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match required type: %s", name, STRINGIFY(T))

#define ComponentInitialize(ref, name, ...)																	\
	ref->BackendInitialiseChild(name, this, SableUI::PackStyles(__VA_ARGS__))

#define ComponentGainRefWithInit(name, T, ref, initLines, ...)												\
	ComponentGainRefNoInit(name, T, ref);																	\
	initLines;																								\
	ComponentInitialize(ref, name, __VA_ARGS__)

// Without SableUI::PackStyles()
#define ComponentInitializeWithStyle(ref, name, style)														\
	ref->BackendInitialiseChild(name, this, style)

#define ComponentGainRefWithStyle(name, T, ref, style)														\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = AddComponent(name);									\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match requried type: %s", name, STRINGIFY(T));		\
	ref->BackendInitialiseChild(name, this, style)

#define ComponentGainRefWithInitWithStyle(name, T, ref, initLines, style)									\
	ComponentGainRefNoInit(name, T, ref);																	\
	initLines;																								\
	ComponentInitializeWithStyle(ref, name, style)

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

// Horizontal splitter
#define HSplitter()		if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::HORIZONTAL); true)
// Vertical splitter
#define VSplitter()		if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::VERTICAL); true)

#define EmptyPanel()	SableUI::AddPanel()
#define Panel(name)		SableUI::AddPanel()->AttachComponent(name)->BackendInitialisePanel()

#define PanelGainRefNoInit(name, T, ref)																	\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = SableUI::AddPanel()->AttachComponent(name);			\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match requried type: %s", name, STRINGIFY(T))

#define PanelGainRef(name, T, ref)																			\
	PanelGainRefNoInit(name, T, ref);																		\
	CONCAT(_comp_, __LINE__)->BackendInitialisePanel()

#define PanelGainRefWithInit(name, T, ref, initLines)														\
	PanelGainRefNoInit(name, T, ref);																		\
	initLines;																								\
	CONCAT(_comp_, __LINE__)->BackendInitialisePanel();

#include <SableUI/components/button.h>
#include <SableUI/components/checkbox.h>

// Button component
#define Button(label, callback, ...)																		\
	ComponentGainRefWithInitWithStyle("sableui_button", SableUI::Button, CONCAT(_button_, __LINE__),		\
		CONCAT(_button_, __LINE__)->Init(label, callback, SableUI::PackStyles(__VA_ARGS__)),				\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__)))

// Checkbox with State<bool> - auto-syncing to parent
#define CheckboxState(label, checked, ...)																	\
	ComponentGainRefWithInitWithStyle("sableui_checkbox", SableUI::Checkbox, CONCAT(_checkbox_, __LINE__),	\
		CONCAT(_checkbox_, __LINE__)->Init(checked, label, nullptr, SableUI::PackStyles(__VA_ARGS__)),		\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__)))

// Checkbox with callback
#define Checkbox(label, checked, onChange, ...)																\
	ComponentGainRefWithInitWithStyle("sableui_checkbox", SableUI::Checkbox, CONCAT(_checkbox_, __LINE__),	\
		CONCAT(_checkbox_, __LINE__)->Init(checked, label, onChange, SableUI::PackStyles(__VA_ARGS__)),		\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__)))

#define TabUpdateHandler(tabContext)																		\
	if (tabContext.changed)																					\
	{																										\
		needsRerender = true;																				\
		tabContext.changed = false;																			\
	}

#include <SableUI/components/text_field.h>
// Single-line input field
#define InputField(state, ...)																				\
    ComponentGainRefWithInitWithStyle("sableui_text_field", SableUI::TextFieldComponent,					\
        CONCAT(_input_field_, __LINE__),																	\
        CONCAT(_input_field_, __LINE__)->Init(state, SableUI::PackStyles(__VA_ARGS__), false),				\
        SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__)))

// Multi-line text field
#define TextField(state, ...)																				\
    ComponentGainRefWithInitWithStyle("sableui_text_field", SableUI::TextFieldComponent,					\
        CONCAT(_text_field_, __LINE__),																		\
        CONCAT(_text_field_, __LINE__)->Init(state, SableUI::PackStyles(__VA_ARGS__), true),				\
        SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__)))

// Visual horizontal splitter element
#define SplitterHorizontal(...)																				\
	Rect(mx(2), my(4), h(1), w_fill, bg(70, 70, 70) __VA_ARGS__)

// Visual vertical splitter element
#define SplitterVertical(...)																				\
	Rect(my(4), mx(2), h_fill, w(1), bg(70, 70, 70) __VA_ARGS__)

// Text with splitters either side
#define TextSeperator(label)																				\
	Div(left_right, h_fit, w_fill, centerY, mt(8))															\
	{																										\
		Rect(mx(2), h(1), w(6), bg(70, 70, 70), centerY);													\
		Text(label, w_fit, wrapText(false), mx(2), mb(4));													\
		Rect(mx(2), h(1), w_fill, bg(70, 70, 70), centerY);													\
	}
