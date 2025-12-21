#pragma once
#include <SableUI/window.h>
#include <SableUI/component.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h>
#include <SableUI/panel.h>
#include <SableUI/renderer.h>
#include <SableUI/text.h>
#include <SableUI/utils.h>
#include <SableUI/console.h>
#include <string>
#include <cstdint>

/* non-macro user api */
namespace SableUI
{
	struct ElementInfo;
	class Element;

	void PreInit(int argc, char** argv);
	void SetBackend(const Backend& backend);

	Window* Initialise(const char* name = "SableUI", int width = 800, int height = 600, int x = -1, int y = -1);
	Window* CreateSecondaryWindow(const char* name = "Unnamed window", int width = 800, int height = 600, int x = -1, int y = -1);
	void Shutdown();

	bool PollEvents();
	bool WaitEvents();
	bool WaitEventsTimeout(double timeout);

	void Render();

	void PostEmptyEvent();

	void SetElementBuilderContext(RendererBackend* renderer, Element* rootElement, bool isVirtual);
	Element* GetCurrentElement();
	VirtualNode* GetVirtualRootNode();

	SplitterPanel* StartSplitter(PanelType orientation);
	void EndSplitter();
	ContentPanel* AddPanel();
	
	void StartDivVirtual(const ElementInfo& info = {}, BaseComponent* child = nullptr);
	void EndDivVirtual();
	void AddRectVirtual(const ElementInfo& info = {});
	void AddImageVirtual(const std::string& path, const ElementInfo& info = {});
	void AddTextVirtual(const SableString& text, const ElementInfo& info = {});

	void StartDiv(const ElementInfo& info = {}, SableUI::BaseComponent* child = nullptr);
	void EndDiv();
	void AddRect(const ElementInfo& info = {});
	void AddImage(const std::string& path, const ElementInfo& info = {});
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
}

/* scoped RAII rect guard api */
#define Rect(...) AddRect(SableUI::ElementInfo{} __VA_ARGS__)
#define Div(...) if (SableUI::DivScope CONCAT(_div_guard_, __LINE__)(SableUI::ElementInfo{} __VA_ARGS__); true)
#define Image(path, ...) AddImage(path, SableUI::ElementInfo{} __VA_ARGS__)
#define Text(text, ...) AddText(text, SableUI::ElementInfo{} __VA_ARGS__)

#define STRINGIFY(x) #x
#define style(...) SableUI::ElementInfo{} __VA_ARGS__
#define Component(name, ...) AddComponent(name)->BackendInitialiseChild(name, this, style(__VA_ARGS__))
#define ComponentGainBaseRef(name, ref, ...)																\
	SableUI::BaseComponent* ref = AddComponent(name);														\
	ref->BackendInitialiseChild(name, this, style(__VA_ARGS__))

#define ComponentGainRef(name, T, ref, ...)																	\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = AddComponent(name);									\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match requried type: %s", name, STRINGIFY(T));		\
	ref->BackendInitialiseChild(name, this, style(__VA_ARGS__))

#define ComponentGainRefNoInit(name, T, ref, ...)															\
	T* ref = nullptr;																						\
	SableUI::BaseComponent* CONCAT(_comp_, __LINE__) = AddComponent(name);									\
	if (dynamic_cast<T*>(CONCAT(_comp_, __LINE__)) != nullptr)												\
		ref = dynamic_cast<T*>(CONCAT(_comp_, __LINE__));													\
	else																									\
		SableUI_Runtime_Error("Component '%s' does not match required type: %s", name, STRINGIFY(T))

#define ComponentInitialize(ref, name, ...)																	\
	ref->BackendInitialiseChild(name, this, style(__VA_ARGS__))

#define ComponentGainRefWithInit(name, T, ref, initLines, ...)												\
	ComponentGainRefNoInit(name, T, ref, __VA_ARGS__);														\
	initLines;																								\
	ComponentInitialize(ref, name, __VA_ARGS__)

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

inline constexpr SableUI::Colour rgb(uint8_t r, uint8_t g, uint8_t b) {
	return SableUI::Colour{ r, g, b, 255 };
}

inline constexpr SableUI::Colour rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return SableUI::Colour{ r, g, b, a };
}

/* style modifiers */
#define ID(value)			.setID(value)
#define bg(...)				.setBgColour(SableUI::Colour(__VA_ARGS__))
#define w(value)			.setWidth(value).setMinWidth(value)
#define minW(value)			.setMinWidth(value)
#define maxW(value)			.setMaxWidth(value)
#define h(value)			.setHeight(value).setMinHeight(value)
#define minH(value)			.setMinHeight(value)
#define maxH(value)			.setMaxHeight(value)
#define w_fill				.setWType(SableUI::RectType::Fill)
#define h_fill				.setHType(SableUI::RectType::Fill)
#define w_fixed				.setWType(SableUI::RectType::Fixed)
#define h_fixed				.setHType(SableUI::RectType::Fixed)
#define w_fit				.setWType(SableUI::RectType::FitContent)
#define h_fit				.setHType(SableUI::RectType::FitContent)

#define m(value)			.setMargin(value)
#define mx(value)			.setMarginX(value)
#define my(value)			.setMarginY(value)
#define mt(value)			.setMarginTop(value)
#define mb(value)			.setMarginBottom(value)
#define ml(value)			.setMarginLeft(value)
#define mr(value)			.setMarginRight(value)

#define p(value)			.setPaddingX(value).setPaddingY(value)
#define px(value)			.setPaddingX(value)
#define py(value)			.setPaddingY(value)
#define pt(value)			.setPaddingTop(value)
#define pb(value)			.setPaddingBottom(value)
#define pl(value)			.setPaddingLeft(value)
#define pr(value)			.setPaddingRight(value)

#define fontSize(value)		.setFontSize(value)
#define lineHeight(value)	.setLineHeight(value)

#define centerX				.setCenterX(true)
#define centerY				.setCenterY(true)
#define centerXY			.setCenterX(true).setCenterY(true)
#define rounded(value)		.setBorderRadius(value)
#define overflow_hidden		.setClipChildren(true)

#define left_right			.setLayoutDirection(SableUI::LayoutDirection::LEFT_RIGHT)
#define right_left			.setLayoutDirection(SableUI::LayoutDirection::RIGHT_LEFT)
#define up_down				.setLayoutDirection(SableUI::LayoutDirection::UP_DOWN)
#define down_up				.setLayoutDirection(SableUI::LayoutDirection::DOWN_UP)

#define textColour(...)		.setTextColour(SableUI::Colour(__VA_ARGS__))
#define justify_left		.setJustification(SableUI::TextJustification::Left)
#define justify_center		.setJustification(SableUI::TextJustification::Center)
#define justify_right		.setJustification(SableUI::TextJustification::Right)
#define absolutePos(x, y)	.setAbsolutePosition(x, y)
#define dir(value)			.setLayoutDirection(value)
#define wrapText(v)			.setTextWrap(v)

#define onHover(...)							.setOnHover(__VA_ARGS__)
#define onHoverExit(...)						.setOnHoverExit(__VA_ARGS__)
#define onClick(...)							.setOnClick(__VA_ARGS__)
#define onSecondaryClick(...)					.setOnSecondaryClick(__VA_ARGS__)
#define onDoubleClick(...)						.setOnDoubleClick(__VA_ARGS__)

#define HSplitter()								if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::HORIZONTAL); true)
#define VSplitter()								if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::VERTICAL); true)

#define EmptyPanel()							SableUI::AddPanel()
#define Panel(name)								SableUI::AddPanel()->AttachComponent(name)->BackendInitialisePanel();

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

#define Button(label, callback, ...)																		\
	ComponentGainRefWithInit("Button", SableUI::Button, CONCAT(_button_, __LINE__),							\
		CONCAT(_button_, __LINE__)->Init(label, callback), __VA_ARGS__)

#define ButtonWithVariant(label, callback, variant, ...)													\
	ComponentGainRefWithInit("Button", SableUI::Button, CONCAT(_button_, __LINE__),							\
		CONCAT(_button_, __LINE__)->Init(label, callback, variant), __VA_ARGS__)

#define ButtonComponentRef(ref, label, callback, ...)														\
	ComponentGainRefWithInit("Button", SableUI::Button, ref,												\
		ref->Init(label, callback), __VA_ARGS__)

#include <SableUI/components/checkbox.h>

#define Checkbox(label, checked, onChange, ...)																\
	ComponentGainRefWithInit("Checkbox", SableUI::Checkbox, CONCAT(_checkbox_, __LINE__),					\
		CONCAT(_checkbox_, __LINE__)->Init(label, checked, onChange), __VA_ARGS__)

#define CheckboxComponentRef(ref, label, checked, onChange, ...)											\
	ComponentGainRefWithInit("Checkbox", SableUI::Checkbox, ref,											\
		ref->Init(label, checked, onChange), __VA_ARGS__)

#define TabUpdateHandler(tabContext)	\
	if (tabContext.changed)				\
	{									\
		needsRerender = true;			\
		tabContext.changed = false;		\
	}

// Splitter components
#define SplitterHorizontal(...) \
	Div(mx(2) my(4) h(1) w_fill bg(70, 70, 70) __VA_ARGS__)

#define SplitterVertical(...) \
	Div(my(4) mx(2) h_fill w(1) bg(70, 70, 70) __VA_ARGS__)

#define SplitterWithText(label, ...)						\
	Div(left_right h_fit w_fill centerY mt(8))				\
	{														\
		Div(mx(2) h(1) w(10) bg(70, 70, 70) centerY);		\
		Text(label, w_fit wrapText(false) mx(4) mb(4));		\
		Div(mx(2) h(1) w_fill bg(70, 70, 70) centerY);		\
	}

#define SplitterWithTextCenter(label, ...)					\
	Div(left_right h_fit w_fill centerY mt(8))				\
	{														\
		Div(mx(2) h(1) w_fill bg(70, 70, 70) centerY);		\
		Text(label, w_fit wrapText(false) mx(4) mb(4));		\
		Div(mx(2) h(1) w_fill bg(70, 70, 70) centerY);		\
	}
