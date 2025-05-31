#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

void SableUI::Renderer::DrawWindowBorder()
{
    static int borderWidth = 1;

    uint32_t* surfacePixels = static_cast<uint32_t*>(texture.pixels);

    /* use std::fill to efficiently draw the border */
    uint32_t* topStart = surfacePixels;
    std::fill(topStart, topStart + texture.width, 0xFF333333);

    uint32_t* bottomStart = surfacePixels + (texture.height - borderWidth) * texture.width;
    std::fill(bottomStart, bottomStart + texture.width, 0xFF333333);

    /* draw left and right borders */
    for (int i = 0; i < texture.height; i++)
    {
        uint32_t* startL = surfacePixels + i * texture.width;
		std::fill(startL, startL + borderWidth, 0xFF333333);

        uint32_t* startR = surfacePixels + i * texture.width + texture.width - borderWidth;
		std::fill(startR, startR + borderWidth, 0xFF333333);
    }
}

void SableUI::Renderer::Flush()
{
    drawStack.clear();
}

void SableUI::Renderer::Draw(std::unique_ptr<SableUI::DrawableBase> drawable)
{
    drawStack.push_back(std::move(drawable));
}

void SableUI::Renderer::Draw()
{
    if (texture.pixels == nullptr)
    {
		SableUI_Error("Renderer surface not set!");
		return;
    }

    std::sort(drawStack.begin(), drawStack.end(), [](const std::unique_ptr<SableUI::DrawableBase>& a, const std::unique_ptr<SableUI::DrawableBase>& b) {
        return a->z < b->z;
    });

    if (drawStack.size() == 0) return;

    /* iterate through queue and draw all types of drawables */
    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            drawable->Draw(&texture);
        }
    }

    /* draw window border after queue is drawn */
    DrawWindowBorder();

    drawStack.clear();

    texture.Update();
}

SableUI::BaseElement* SableUI::Renderer::CreateElement(const std::string& name)
{
    for (BaseElement* element : elements)
    {
        if (element->name == name)
        {
            SableUI_Error("Element already exists: %s", name.c_str());
            return nullptr;
        }
    }

    BaseElement* element = new BaseElement(name, this);
    elements.push_back(element);

    return element;
}

SableUI::BaseElement* SableUI::Renderer::GetElement(const std::string& name)
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

SableUI::Renderer::~Renderer()
{
    for (BaseElement* element : elements)
	{
		delete element;
	}

    elements.clear();
}