#pragma once
#include "SableUI/window.h"

/* non-macro user api */
namespace SableUI
{
	struct ElementInfo;
	class Element;

	void PreInit(int argc, char** argv);
	void SetBackend(const Backend& backend);

	Window* Initialise(const char* name = "SableUI", int width = 800, int height = 600, int x = -1, int y = -1);
	void Shutdown();

	Window* CreateSecondaryWindow(const char* name = "Unnamed window", int width = 800, int height = 600, int x = -1, int y = -1);

	bool PollEvents();
	void Render();

	void SetElementBuilderContext(Renderer* renderer, Element* rootElement, bool isVirtual);
	void SetContext(Window* window);
	Element* GetCurrentElement();
	VirtualNode* GetVirtualRootNode();
	
	SplitterPanel* StartSplitter(PanelType orientation);
	void EndSplitter();
	ContentPanel* AddPanel();
	
	void StartDivVirtual(const ElementInfo& info = {}, BaseComponent* child = nullptr);
	void EndDivVirtual();
	void AddRectVirtual(const ElementInfo& info = {});
	void AddImageVirtual(const std::string& path, const ElementInfo& info = {});
	void AddTextVirtual(const std::string& text, const ElementInfo& info = {});
	void AddTextU32Virtual(const SableString& text, const ElementInfo& info = {});

	void StartDiv(const ElementInfo& info = {}, SableUI::BaseComponent* child = nullptr);
	void EndDiv();
	void AddRect(const ElementInfo& info = {});
	void AddImage(const std::string& path, const ElementInfo& info = {});
	void AddText(const std::string& text, const ElementInfo& info = {});
	void AddTextU32(const SableString& text, const ElementInfo& info = {});

	void* GetCurrentContext();

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
#define TextU32(text, ...) AddTextU32(text, SableUI::ElementInfo{} __VA_ARGS__)

#define STRINGIFY(x) #x
#define style(...) SableUI::ElementInfo{} __VA_ARGS__
#define Component(T, info, ...) AddComponent<T>(__VA_ARGS__)->BackendInitialiseChild(STRINGIFY(T), this, style(info))

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

/*  Box Model
	/-----------------------------------------\
	| Margin (Top, Bottom, Left, Right)       |
	| |------------------------------------ | |
	| | Padding (Top, Bottom, Left, Right)  | |
	| | |-------------------------------- | | |
	| | | Content (rect)                  | | |
	| | |-------------------------------- | | |
	| |------------------------------------ | |
	\-----------------------------------------/ */

#define rgb(r, g, b) SableUI::Colour(r, g, b)

/* style modifiers */
#define id(value)			.setID(value)
#define bg(...)				.setBgColour(SableUI::Colour(__VA_ARGS__))
#define w(value)			.setWidth(value)
#define minW(value)			.setMinWidth(value)
#define maxW(value)			.setMaxWidth(value)
#define h(value)			.setHeight(value)
#define minH(value)			.setMinHeight(value)
#define maxH(value)			.setMaxHeight(value)
#define w_fill				.setWType(SableUI::RectType::FILL)
#define h_fill				.setHType(SableUI::RectType::FILL)
#define w_fixed				.setWType(SableUI::RectType::FIXED)
#define h_fixed				.setHType(SableUI::RectType::FIXED)
#define w_fit				.setWType(SableUI::RectType::FIT_CONTENT)
#define h_fit				.setHType(SableUI::RectType::FIT_CONTENT)

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

#define left_right			.setLayoutDirection(SableUI::LayoutDirection::LEFT_RIGHT)
#define right_left			.setLayoutDirection(SableUI::LayoutDirection::RIGHT_LEFT)
#define up_down				.setLayoutDirection(SableUI::LayoutDirection::UP_DOWN)
#define down_up				.setLayoutDirection(SableUI::LayoutDirection::DOWN_UP)

#define textColour(...)		.setTextColour(SableUI::Colour(__VA_ARGS__))
#define justify_left		.setJustification(SableUI::TextJustification::Left)
#define justify_center		.setJustification(SableUI::TextJustification::Center)
#define justify_right		.setJustification(SableUI::TextJustification::Right)

#define dir(value)			.setLayoutDirection(value)

#define useState(variableName, setterName, T, initialValue)				\
    T variableName = initialValue;										\
    SableUI::StateSetter<T> setterName = SableUI::StateSetter<T>(		\
        [this](T const& val) {											\
            if (this->variableName == val) return;						\
            this->variableName = val;									\
            this->needsRerender = true;									\
        });																\
    struct __StateReg_##variableName {									\
        __StateReg_##variableName(SableUI::BaseComponent* comp, T* var)	\
        { comp->RegisterState(var); }									\
    } __stateReg_##variableName{this, &variableName};

#define onHover(callback)           .setOnHover(callback)
#define onHoverExit(callback)       .setOnHoverExit(callback)
#define onClick(callback)           .setOnClick(callback)
#define onSecondaryClick(callback)  .setOnSecondaryClick(callback)

#define HSplitter()                 if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::HORIZONTAL); true)
#define VSplitter()                 if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::VERTICAL); true)

#define Panel()                     SableUI::AddPanel();
#define PanelWith(T, ...)           SableUI::AddPanel()->AttachComponent<T>(__VA_ARGS__);