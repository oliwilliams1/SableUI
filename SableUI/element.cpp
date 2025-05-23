#include "SableUI/element.h"

static SableUI::Renderer* renderer = nullptr;

using namespace SableUI;

void SableUI::BaseElement::SetRect(const SableUI::rect& rect)
{
	this->drawableRect = rect;
	this->bgDrawable.Update(drawableRect, bgColour, 0.0f, true);
}

void SableUI::BaseElement::SetInfo(const ElementInfo& info)
{
    this->name = info.name;
    this->xOffset = info.xOffset;
    this->yOffset = info.yOffset;
    this->width = info.width;
    this->height = info.height;
    this->padding = info.padding;
    this->wType = info.wType;
    this->hType = info.hType;
    this->bgColour = info.bgColour;
}

void SableUI::BaseElement::Render()
{
	if (renderer == nullptr) renderer = &SableUI::Renderer::Get();

	renderer->Draw(std::make_unique<SableUI_Drawable::Rect>(bgDrawable));

	AdditionalRender();
}

std::vector<BaseElement*> ElementArena::elements;

BaseElement* ElementArena::CreateElement(const std::string& name)
{
    for (BaseElement* element : elements)
    {
        if (element->name == name)
        {
            SableUI_Error("Element already exists: %s", name.c_str());
            return nullptr;
        }
    }

    BaseElement* element = new BaseElement(name);
    elements.push_back(element);

    SableUI_Log("Created element: %s", name.c_str());

    return element;
}

BaseElement* ElementArena::GetElement(const std::string& name)
{
    for (BaseElement* element : elements)
    {
        if (element->name == name)
        {
            return element;
        }
    }

    return nullptr;
}

void ElementArena::ShutdownArena()
{
    for (BaseElement* element : elements)
    {
        delete element;
    }

    elements.clear();
}
