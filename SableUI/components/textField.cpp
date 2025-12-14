#include <SableUI/components/textField.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>

void SableUI::TextField::Layout()
{
    Div(ID("TextField") w_fill h_fit p(8) bg(40, 40, 40) rounded(4))
    {
        SableString displayText = textVal;

        if (isFocused)
            displayText = displayText + "|";
        else
            displayText = displayText;

        Text(displayText, ID("TextFieldText") textColour(220, 220, 220));
    }
}

void SableUI::TextField::OnUpdate(const UIEventContext& ctx)
{
    if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
    {
        Element* el = GetElementById("TextField");
        if (!el) return;

        if (RectBoundingBox(el->rect, ctx.mousePos))
            setIsFocused(true);
        else
        {
            setIsFocused(false);
            RemoveQueueFromContext(queue);
        }
    }

    if (!isFocused) return;

    if (ctx.typedChar != '\0')
        setTextVal(textVal + (char32_t)ctx.typedChar);

    if (ctx.keyPressedEvent.test(SABLE_KEY_BACKSPACE))
        setTextVal(textVal.substr(0, textVal.size() - 1));

    if (ctx.keyPressedEvent.test(SABLE_KEY_ENTER))
        setTextVal(textVal + '\n');
}

void SableUI::TextField::OnPostLayoutUpdate(const UIEventContext& ctx)
{
    if (!isFocused || !m_window) return;

    UseCustomLayoutContext(queue, m_window, m_window->GetSurface())
    {
        Element* text = GetElementById("TextFieldText");
        if (!text) return;

        Rect highlightRect = text->rect;
        highlightRect.h += text->lineHeight * text->fontSize;

        queue->AddRect(highlightRect, Colour(100, 180, 255, 120));
    }
}