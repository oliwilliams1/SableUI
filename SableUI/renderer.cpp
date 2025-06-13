#include <algorithm>
#include <string>
#include <set>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

void SableUI::Renderer::Flush()
{
    drawStack.clear();
}

void SableUI::Renderer::Draw(DrawableBase* drawable)
{
    drawStack.push_back(drawable);
}

void SableUI::Renderer::Draw()
{
    if (renderTarget.target == TargetType::TEXTURE) renderTarget.Bind();

    std::sort(drawStack.begin(), drawStack.end(), [](const DrawableBase* a, const DrawableBase* b) {
        return a->m_zIndex < b->m_zIndex;
    });

    if (drawStack.size() == 0) return;

    std::set<unsigned int> drawnUUIDs;

    /* iterate through queue and draw all types of drawables */
    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            if (drawnUUIDs.find(drawable->uuid) != drawnUUIDs.end()) continue;
            drawnUUIDs.insert(drawable->uuid);
            drawable->Draw(&renderTarget);
        }
    }

    /* draw window border after queue is drawn */
    DrawWindowBorder(&renderTarget);

    drawStack.clear();
}

SableUI::Element* SableUI::Renderer::CreateElement(const std::string& name, ElementType type)
{
    for (Element* element : elements)
    {
        if (element->name == name)
        {
            SableUI_Error("Element already exists: %s", name.c_str());
            return nullptr;
        }
    }

    Element* element = new Element(name, this, type);
    elements.push_back(element);

    return element;
}

SableUI::Element* SableUI::Renderer::GetElement(const std::string& name)
{
    for (Element* element : elements)
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
    for (Element* element : elements)
	{
		delete element;
	}

    elements.clear();
}