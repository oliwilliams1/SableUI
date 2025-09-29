#include "SableUI/element.h"
#include "SableUI/component.h"
#define SABLEUI_SUBSYSTEM "SableUI::Element"

static int n_elements = 0;

static std::size_t ComputeHash(const SableUI::VirtualNode* vnode)
{
    std::size_t h = 0;

    h ^= std::hash<int>()((int)vnode->type);

    if (vnode->uniqueTextOrPath.size() != 0)
        h ^= std::hash<std::string>()(std::string(vnode->uniqueTextOrPath.begin(), vnode->uniqueTextOrPath.end()));

    h ^= std::hash<int>()((int)vnode->info.wType);
    h ^= std::hash<int>()((int)vnode->info.hType);
    h ^= std::hash<int>()(vnode->info.minWidth ^ vnode->info.minHeight ^
        vnode->info.maxWidth    ^ vnode->info.maxHeight ^
        vnode->info.marginTop   ^ vnode->info.marginBottom ^
        vnode->info.marginLeft  ^ vnode->info.marginRight ^
        vnode->info.paddingTop  ^ vnode->info.paddingBottom ^
        vnode->info.paddingLeft ^ vnode->info.paddingRight);

    h ^= std::hash<int>()((int)vnode->info.layoutDirection);

    return h;
}

static std::size_t ComputeHash(const SableUI::Element* elem)
{
    std::size_t h = 0;
    h ^= std::hash<int>()((int)elem->type);

    if (elem->uniqueTextOrPath.size() != 0)
        h ^= std::hash<std::string>()(std::string(elem->uniqueTextOrPath.begin(), elem->uniqueTextOrPath.end()));

    h ^= std::hash<int>()((int)elem->wType);
    h ^= std::hash<int>()((int)elem->hType);
    h ^= std::hash<int>()(elem->minWidth ^ elem->minHeight ^
        elem->maxWidth      ^ elem->maxHeight ^
        elem->marginTop     ^ elem->marginBottom ^
        elem->marginLeft    ^ elem->marginRight ^
        elem->paddingTop    ^ elem->paddingBottom ^
        elem->paddingLeft   ^ elem->paddingRight);

    h ^= std::hash<int>()((int)elem->layoutDirection);

    return h;
}

SableUI::Child::~Child()
{
    if (type == ChildType::ELEMENT)
    {
        delete element;
    }
    else if (type == ChildType::COMPONENT)
    {
        delete component;
    }
}

/* child struct */
SableUI::Child::operator SableUI::Element* ()
{
    if (type == ChildType::ELEMENT)
        return element;
    else if (type == ChildType::COMPONENT)
        return component->GetRootElement();

    SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
	return nullptr;
}

SableUI::Element::Element()
{
    n_elements++;
}

SableUI::Element::Element(Renderer* renderer, ElementType type)
{
    n_elements++;
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
        drawable = new DrawableRect();
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

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::SetInfo(const ElementInfo& info)
{
    this->ID                        = info.id;
    this->width                     = info.width;
    this->minWidth                  = info.minWidth;
    this->maxWidth                  = info.maxWidth;
    this->height                    = info.height;
    this->minHeight                 = info.minHeight;
    this->maxHeight                 = info.maxHeight;
    this->marginTop                 = info.marginTop;
    this->marginBottom              = info.marginBottom;
    this->marginLeft                = info.marginLeft;
    this->marginRight               = info.marginRight;
    this->paddingTop                = info.paddingTop;
    this->paddingBottom             = info.paddingBottom;
    this->paddingLeft               = info.paddingLeft;
    this->paddingRight              = info.paddingRight;
    this->centerX                   = info.centerX;
    this->centerY                   = info.centerY;
    this->wType                     = info.wType;
    this->hType                     = info.hType;
    this->bgColour                  = info.bgColour;
    this->layoutDirection           = info.layoutDirection;
    this->m_onHoverFunc             = info.onHoverFunc;
    this->m_onHoverExitFunc         = info.onHoverExitFunc;
    this->m_onClickFunc             = info.onClickFunc;
    this->m_onSecondaryClickFunc    = info.onSecondaryClickFunc;
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
            renderer->Draw(drRect);
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
            renderer->Draw(drImage);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::TEXT:
    {
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            drawable->setZ(z);
            renderer->Draw(drText);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::DIV:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drawable->setZ(z);
            renderer->Draw(drRect);
        }
        else
        {
			SableUI_Error("Dynamic cast failed");
        }

        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->Render(z + 1);
        }
        break;
    }

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::AddChild(Element* child)
{
    if (type != ElementType::DIV) { SableUI_Error("Cannot add child to element not of type div"); return; };
    
    children.emplace_back(new Child(child));
}

void SableUI::Element::AddChild(Child* child)
{
    if (type != ElementType::DIV) { SableUI_Error("Cannot add child to element not of type div"); return; };

    children.emplace_back(child);
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

void SableUI::Element::SetText(const SableString& text, int fontSize, float lineHeight)
{
    if (type != ElementType::TEXT) SableUI_Error("Cannot set text on element not of type text");

    if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
    {
        drText->m_text.SetContent(text, drawable->m_rect.w, fontSize, lineHeight);
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

int SableUI::Element::GetMinWidth()
{
    int calculatedMinWidth = minWidth;

    if (type == ElementType::DIV)
    {
        bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->marginLeft + childElement->marginRight;
                calculatedMinWidth = std::max(calculatedMinWidth, childTotalWidth);
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->marginLeft + childElement->marginRight;
                calculatedMinWidth += childTotalWidth;
            }
        }
    }
    else if (type == ElementType::TEXT)
    {
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            calculatedMinWidth = std::max(calculatedMinWidth, drText->m_text.GetMinWidth());
        }
    }
    else
    {
        calculatedMinWidth = std::max(calculatedMinWidth, width);
    }

    return calculatedMinWidth + paddingLeft + paddingRight;
}

int SableUI::Element::GetMinHeight()
{
    int calculatedMinHeight = minHeight;

    if (type == ElementType::DIV)
    {
        bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->marginTop + childElement->marginBottom;
                calculatedMinHeight += childTotalHeight;
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->marginTop + childElement->marginBottom;
                calculatedMinHeight = std::max(calculatedMinHeight, childTotalHeight);
            }
        }
    }
    else if (type == ElementType::TEXT)
    {
        if (measuredHeight > 0)
        {
            calculatedMinHeight = measuredHeight;
        }
        else
        {
            if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
            {
                calculatedMinHeight = std::max(calculatedMinHeight, drText->m_text.GetUnwrappedHeight());
            }
        }
    }
    else
    {
        calculatedMinHeight = std::max(calculatedMinHeight, height);
    }

    return calculatedMinHeight + paddingTop + paddingBottom;
}

void SableUI::Element::LayoutChildren()
{
    if (type != ElementType::DIV) return;
    if (children.empty()) return;

    bool layoutDirty = false;
    int totalChildHeight = 0;
    int totalChildWidth = 0;

    rect.w = std::max(rect.w, GetMinWidth());
    rect.h = std::max(rect.h, GetMinHeight());

    // Calculate container area (no padding)
    ivec2 containerSize = { rect.w, rect.h };

    if (containerSize.x <= 0 || containerSize.y <= 0)
    {
        SableUI_Warn("Container has zero or negative size ID: \"%s\"", ID.c_str());
        // Set all children to zero size
        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ rect.x, rect.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    // Calculate content area (after padding)
    ivec2 contentAreaPosition = { rect.x + paddingLeft, rect.y + paddingTop };
    ivec2 contentAreaSize = { std::max(0, containerSize.x - paddingLeft - paddingRight),
                                     std::max(0, containerSize.y - paddingTop - paddingBottom) };

    if (contentAreaSize.x <= 0 || contentAreaSize.y <= 0)
    {
        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ contentAreaPosition.x, contentAreaPosition.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
    bool isReverseFlow = (layoutDirection == LayoutDirection::DOWN_UP || layoutDirection == LayoutDirection::RIGHT_LEFT);

    int totalFixedMainAxis = 0;
    int totalMarginMainAxis = 0;
    int fillMainAxisCount = 0;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;

        if (isVerticalFlow)
        {
            totalMarginMainAxis += childElement->marginTop + childElement->marginBottom;

            if (childElement->hType == RectType::FIXED)
            {
                totalFixedMainAxis += std::min(std::max(childElement->height, childElement->minHeight), childElement->maxHeight > 0 ? childElement->maxHeight : childElement->height);
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                int minHeight = childElement->GetMinHeight();
                totalFixedMainAxis += std::min(std::max(0, minHeight), childElement->maxHeight > 0 ? childElement->maxHeight : minHeight);
            }
            else if (childElement->hType == RectType::FILL)
            {
                fillMainAxisCount++;
            }
        }
        else
        {
            totalMarginMainAxis += childElement->marginLeft + childElement->marginRight;

            if (childElement->wType == RectType::FIXED)
            {
                totalFixedMainAxis += std::min(std::max(childElement->width, childElement->minWidth), childElement->maxWidth > 0 ? childElement->maxWidth : childElement->width);
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                int minWidth = childElement->GetMinWidth();
                totalFixedMainAxis += std::min(std::max(0, minWidth), childElement->maxWidth > 0 ? childElement->maxWidth : minWidth);
            }
            else if (childElement->wType == RectType::FILL)
            {
                fillMainAxisCount++;
            }
        }
    }

    int availableMainAxis = isVerticalFlow ? contentAreaSize.y : contentAreaSize.x;
    int remainingMainAxis = availableMainAxis - totalFixedMainAxis - totalMarginMainAxis;
    int fillMainAxisSize = (fillMainAxisCount > 0) ? std::max(0, remainingMainAxis / fillMainAxisCount) : 0;
    int fillMainAxisRemainder = (fillMainAxisCount > 0) ? remainingMainAxis % fillMainAxisCount : 0;

    ivec2 cursor = isReverseFlow ?
        (isVerticalFlow ? ivec2(contentAreaPosition.x, contentAreaPosition.y + contentAreaSize.y) :
            ivec2(contentAreaPosition.x + contentAreaSize.x, contentAreaPosition.y)) :
        contentAreaPosition;

    int distributedRemainder = 0;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;

        int childMarginWidth = childElement->marginLeft + childElement->marginRight;
        int childMarginHeight = childElement->marginTop + childElement->marginBottom;

        int childContentWidth, childContentHeight;

        if (isVerticalFlow)
        {
            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->type == ElementType::TEXT || childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            if (childElement->type == ElementType::TEXT)
            {
                if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
                {
                    int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                    if (newHeight != childElement->height)
                    {
                        childElement->height = newHeight;
                        layoutDirty = true;
                    }
                    childContentHeight = newHeight;
                }
                else
                {
                    childContentHeight = childElement->height;
                }
            }
            else if (childElement->hType == RectType::FIXED)
            {
                childContentHeight = childElement->height;
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->paddingTop - childElement->paddingBottom);
            }
            else
            {
                childContentHeight = fillMainAxisSize;
                if (distributedRemainder < fillMainAxisRemainder)
                {
                    childContentHeight += 1;
                    distributedRemainder++;
                }
            }
            childContentHeight = std::max(childContentHeight, childElement->minHeight);
            childContentHeight = (childElement->maxHeight > 0) ? std::min(childContentHeight, childElement->maxHeight) : childContentHeight;

            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->paddingLeft - childElement->paddingRight);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            childContentWidth = std::max(childContentWidth, childElement->minWidth);
            childContentWidth = (childElement->maxWidth > 0) ? std::min(childContentWidth, childElement->maxWidth) : childContentWidth;
        }
        else
        {
            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->paddingLeft - childElement->paddingRight);
            }
            else
            {
                childContentWidth = fillMainAxisSize;
                if (distributedRemainder < fillMainAxisRemainder)
                {
                    childContentWidth += 1;
                    distributedRemainder++;
                }
            }

            if (childElement->hType == RectType::FIXED)
            {
                childContentHeight = childElement->height;
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->paddingTop - childElement->paddingBottom);
            }
            else
            {
                childContentHeight = std::max(0, contentAreaSize.y - childMarginHeight);
            }
        }

        childContentWidth = std::max(0, childContentWidth);
        childContentHeight = std::max(0, childContentHeight);

        if (childElement->type == ElementType::TEXT && isVerticalFlow)
        {
            if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
            {
                int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                if (newHeight != childElement->height)
                {
                    childElement->height = newHeight;
                    layoutDirty = true;
                }
                childContentHeight = newHeight;

            }
        }

        childElement->measuredHeight = childContentHeight;

        childContentWidth += childElement->paddingLeft + childElement->paddingRight;
        childContentHeight += childElement->paddingTop + childElement->paddingBottom;

        int childTotalWidth = childContentWidth + childMarginWidth;
        int childTotalHeight = childContentHeight + childMarginHeight;

        int childX, childY;

        if (isReverseFlow)
        {
            if (isVerticalFlow) // DOWN_UP
            {
                cursor.y -= childTotalHeight;
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
            }
            else // RIGHT_LEFT
            {
                cursor.x -= childTotalWidth;
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
            }
        }
        else
        {
            if (isVerticalFlow) // UP_DOWN
            {
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
                cursor.y += childTotalHeight;
            }
            else // LEFT_RIGHT
            {
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
                cursor.x += childTotalWidth;
            }
        }

        // Centering logic for single child
        if (children.size() == 1)
        {
            if (childElement->centerX)
            {
                int availableCrossSpace = contentAreaSize.x - childTotalWidth;
                if (availableCrossSpace > 0)
                {
                    childX = contentAreaPosition.x + availableCrossSpace / 2 + childElement->marginLeft;
                }
            }
            if (childElement->centerY)
            {
                int availableCrossSpace = contentAreaSize.y - childTotalHeight;
                if (availableCrossSpace > 0)
                {
                    childY = contentAreaPosition.y + availableCrossSpace / 2 + childElement->marginTop;
                }
            }
        }
        else
        {
            if (isVerticalFlow && childElement->centerX)
            {
                int availableCrossSpace = contentAreaSize.x - childTotalWidth;
                if (availableCrossSpace > 0)
                {
                    childX = contentAreaPosition.x + availableCrossSpace / 2 + childElement->marginLeft;
                }
            }
            else if (!isVerticalFlow && childElement->centerY)
            {
                int availableCrossSpace = contentAreaSize.y - childTotalHeight;
                if (availableCrossSpace > 0)
                {
                    childY = contentAreaPosition.y + availableCrossSpace / 2 + childElement->marginTop;
                }
            }
        }

        Rect childRect = {
            childX,
            childY,
            childContentWidth,
            childContentHeight
        };

        totalChildWidth += childContentWidth + childElement->marginLeft + childElement->marginRight;
        totalChildHeight += childContentHeight + childElement->marginTop + childElement->marginBottom;

        SableUI::Rect childFinalRect = {
            childRect.x,
            childRect.y,
            std::min(childRect.w, (contentAreaPosition.x + contentAreaSize.x) - childRect.x),
            std::min(childRect.h, (contentAreaPosition.y + contentAreaSize.y) - childRect.y)
        };

        childElement->SetRect(childFinalRect);
        childElement->LayoutChildren();
    }

    if (layoutDirty)
    {
        layoutDirty = false;
        SetRect({ rect.x, rect.y, totalChildWidth + paddingLeft + paddingRight, totalChildHeight + paddingTop + paddingBottom });
        LayoutChildren();
    }
}

#include "SableUI/SableUI.h"
void SableUI::Element::BuildRealSubtreeFromVirtual(VirtualNode* vnode, Element* parent)
{
    if (!vnode) return;

    switch (vnode->type)
    {
    case ElementType::DIV:
    {
        StartDiv(vnode->info, vnode->childComp);

        for (VirtualNode* vchild : vnode->children)
            BuildRealSubtreeFromVirtual(vchild, nullptr);

        EndDiv();
        break;
    }

    case ElementType::RECT:
    {
        AddRect(vnode->info);
        break;
    }

    case ElementType::TEXT:
    {
        AddText(std::string(vnode->uniqueTextOrPath.begin(), vnode->uniqueTextOrPath.end()), vnode->info);
        break;
    }

    case ElementType::TEXT_U32:
    {
        AddTextU32(vnode->uniqueTextOrPath, vnode->info);
        break;
    }

    case ElementType::IMAGE:
    {
        AddImage(std::string(vnode->uniqueTextOrPath.begin(), vnode->uniqueTextOrPath.end()), vnode->info);
        break;
    }
    }
}

bool SableUI::Element::Reconcile(VirtualNode* vnode)
{
    if (this->type != vnode->type || ComputeHash(this) != ComputeHash(vnode))
    {
        for (Child* child : children) delete child;
        children.clear();

        SetElementBuilderContext(renderer, this, true);
        BuildRealSubtreeFromVirtual(vnode, this);

        return true;
    }

    size_t minCount = std::min(children.size(), vnode->children.size());
    for (size_t i = 0; i < minCount; i++)
    {
        Element* el = (Element*)*children[i];
        el->Reconcile(vnode->children[i]);
    }

    for (size_t i = minCount; i < vnode->children.size(); i++)
    {
        SetElementBuilderContext(renderer, this, true);
        BuildRealSubtreeFromVirtual(vnode->children[i], this);
    }

    for (size_t i = vnode->children.size(); i < children.size(); i++) {
        delete children[i];
    }
    children.resize(vnode->children.size());

    return false;
}

bool SableUI::Element::el_PropagateComponentStateChanges()
{
    for (Child* child : children)
    {
        bool res = false;
        switch (child->type)
        {
        case ChildType::COMPONENT:
        {
			res = res || child->component->comp_PropagateComponentStateChanges();
            break;
        }
        case ChildType::ELEMENT:
        {
			res = res || child->element->el_PropagateComponentStateChanges();
            break;
        }
        default:
			SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
        }

        if (res) return true;
	}

    return false;
}

void SableUI::Element::HandleHoverEvent(const ivec2& mousePos)
{
    bool wasHovered = isHovered;
    isHovered = RectBoundingBox(rect, mousePos);

    if (isHovered)
    {
        if (m_onHoverFunc) m_onHoverFunc();
    }
    else if (wasHovered && !isHovered)
    {
        if (m_onHoverExitFunc) m_onHoverExitFunc();
    }

    for (Child* child : children)
    {
        Element* el = (Element*)*child;
        el->HandleHoverEvent(mousePos);
    }
}

void SableUI::Element::HandleMouseClickEvent(const MouseButtonState& mouseState)
{
    isHovered = RectBoundingBox(rect, mouseState.pos);
    if (!isHovered) return;
    if (mouseState.LMBEvent == MouseEvent::CLICK)
	{
		if (m_onClickFunc) m_onClickFunc();
	}

    if (mouseState.RMBEvent == MouseEvent::CLICK)
    {
        if (m_onSecondaryClickFunc) m_onSecondaryClickFunc();
    }

    for (Child* child : children)
    {
        Element* el = (Element*)*child;
		el->HandleMouseClickEvent(mouseState);
    }
}

SableUI::Element::~Element()
{
    n_elements--;
    delete drawable;

    for (Child* child : children) delete child;
    children.clear();
}