#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

void SableUI::Renderer::DrawWindowBorder() const
{
    static int borderWidth = 1;

    glColor4ub(51, 51, 51, 255);

    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f - (borderWidth * 2.0f / texture.height));
    glVertex2f(-1.0f, 1.0f - (borderWidth * 2.0f / texture.height));
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f + (borderWidth * 2.0f / texture.height));
    glVertex2f(1.0f, -1.0f + (borderWidth * 2.0f / texture.height));
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(-1.0f + (borderWidth * 2.0f / texture.width), 1.0f);
    glVertex2f(-1.0f + (borderWidth * 2.0f / texture.width), -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(1.0f - (borderWidth * 2.0f / texture.width), 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f - (borderWidth * 2.0f / texture.width), -1.0f);
    glEnd();
}

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
    if (texture.target == TargetType::TEXTURE) texture.Bind();

    std::sort(drawStack.begin(), drawStack.end(), [](const DrawableBase* a, const DrawableBase* b) {
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