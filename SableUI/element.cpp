#include "SableUI/element.h"
#include "SableUI/component.h"

/* child struct */
SableUI::Child::operator SableUI::Element* ()
{
    if (type == ChildType::ELEMENT)
        return element;
    else if (type == ChildType::COMPONENT)
        return component->GetBaseElement();

    SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
	return nullptr;
}

SableUI::Child::~Child()
{
    if (type == ChildType::ELEMENT)
        delete element;
    else if (type == ChildType::COMPONENT)
        delete component;
}

SableUI::Element::Element(Renderer* renderer, ElementType type)
{
    Init(renderer, type);
}

void SableUI::Element::Init(Renderer* renderer, ElementType type)
{
    this->renderer = renderer;
    this->type = type;

    switch (type)
    {
    case ElementType::RECT:
        drawable = new DrawableRect();
        break;

    case ElementType::IMAGE:
        drawable = new DrawableImage();
        break;

    case ElementType::TEXT:
        drawable = new DrawableText();
        break;

    case ElementType::DIV:
        drawable = nullptr;
        break;

    default:
        SableUI_Runtime_Error("Unknown ElementType");
        drawable = nullptr;
        break;
    }
}

void SableUI::Element::SetRect(const Rect& r)
{
    this->rect = r;

    switch (type)
    {
    case ElementType::RECT:
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drRect->Update(rect, bgColour, 0.0f);
            renderer->Draw(drRect);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::IMAGE:
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
        {
            drImage->Update(rect);
            renderer->Draw(drImage);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::TEXT:
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            rect.h = drText->m_text.UpdateMaxWidth(rect.w);
            height = rect.h;
            drText->Update(rect);
            renderer->Draw(drText);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::DIV:
        break;

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::SetInfo(const ElementInfo& info)
{
    this->xOffset = info.xOffset;
    this->yOffset = info.yOffset;
    this->width = info.width;
    this->height = info.height;
    this->padding = info.padding;
    this->wType = info.wType;
    this->hType = info.hType;
    this->centerX = info.centerX;
    this->centerY = info.centerY;
    this->bgColour = info.bgColour;
    this->type = info.type;
    this->layoutDirection = info.layoutDirection;
}

void SableUI::Element::Render(int z)
{
    switch (type)
    {
    case ElementType::RECT:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drawable->setZ(z);
            renderer->Draw(drawable);

            // UpdateChildren();
            for (Child child : children)
            {
                Element* childElement = (Element*)child;
                childElement->Render(z + 1);
            }
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::IMAGE:
    {
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
        {
            drawable->setZ(z);
            renderer->Draw(drawable);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::TEXT:
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            drawable->setZ(z);
            renderer->Draw(drawable);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::DIV:
        // UpdateChildren();
        for (Child child : children)
        {
            Element* childElement = (Element*)child;
            childElement->Render(z + 1);
        }
        break;

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::AddChild(Element* child)
{
    if (type == ElementType::IMAGE || type == ElementType::TEXT) SableUI_Error("Cannot add children to element of type image");
    children.push_back(child);
}

void SableUI::Element::AddChild(BaseComponent* component)
{
    children.push_back(component);
}

void SableUI::Element::SetImage(const std::string& path)
{
    if (type != ElementType::IMAGE) SableUI_Error("Cannot set image on element not of type image");

    if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
    {
        drImage->m_texture.LoadTextureOptimised(path, width, height);
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

void SableUI::Element::SetText(const std::u32string& text, int fontSize, float lineHeight)
{
    if (type != ElementType::TEXT) SableUI_Error("Cannot set text on element not of type text");

    if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
    {
        int reqHeight = drText->m_text.SetContent(text, drawable->m_rect.w, fontSize, lineHeight);
        height = reqHeight;
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

SableUI::Element::~Element()
{
    delete drawable;
}