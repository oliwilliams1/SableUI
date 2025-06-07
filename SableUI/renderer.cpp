#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

void SableUI::Renderer::DrawWindowBorder()
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

void SableUI::Renderer::Draw(std::unique_ptr<SableUI::DrawableBase> drawable)
{
    drawStack.push_back(std::move(drawable));
}

void SableUI::Renderer::Draw()
{
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