#include <SableUI/components/textField.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <SableUI/text.h>
#include <algorithm>
#include <cmath>

static int GetNextWordPos(const SableString& text, int cursorPos, int direction)
{
    for (int i = cursorPos + direction; i < text.size() && i >= 0; i += direction)
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\n')
			return i;

    if (direction == -1)
		return 0;
    else
		return text.size();
}

static void DeleteSelection(SableString& text, int& cursorPos, int& initialCursorPos)
{
    int selStart = std::min(cursorPos, initialCursorPos);
    int selEnd = std::max(cursorPos, initialCursorPos);

    text = text.substr(0, selStart) + text.substr(selEnd);
    cursorPos = selStart;
    initialCursorPos = -1;
}

void SableUI::TextField::Layout()
{
    if (!m_window)
        Text("Component does not have m_window defined", textColour(255, 0, 0) mb(4));

    Div(ID("TextField") w_fill h_fit p(8) bg(40, 40, 40) rounded(4))
    {
        if (textVal.empty() && !isFocused)
            Text("Start writing...", textColour(100, 100, 100));
        else
            Text(textVal, ID("TextFieldText") textColour(220, 220, 220));
    }
}

void SableUI::TextField::OnUpdate(const UIEventContext& ctx)
{
    bool ctrlDown = (ctx.isKeyDown.test(SABLE_KEY_LEFT_CONTROL) || ctx.isKeyDown.test(SABLE_KEY_RIGHT_CONTROL));
    bool shiftDown = (ctx.isKeyDown.test(SABLE_KEY_LEFT_SHIFT) || ctx.isKeyDown.test(SABLE_KEY_RIGHT_SHIFT));
    bool sectionHighlighted = initialCursorPos >= 0 && initialCursorPos != cursorPos;

    if (ctx.keyPressedEvent.test(SABLE_KEY_LEFT_SHIFT) || ctx.keyPressedEvent.test(SABLE_KEY_RIGHT_SHIFT))
        setInitialCursorPos(cursorPos);

    if (ctx.keyPressedEvent.test(SABLE_KEY_ESCAPE))
    {
        if (initialCursorPos == -1)
        {
            setIsFocused(false);
            RemoveQueueFromContext(queue);
        }

        setInitialCursorPos(-1);
    }
    
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

    if (ctx.typedCharBuffer.size() != 0)
    {
        SableString baseText = textVal;
        int newCursor = cursorPos;

        if (sectionHighlighted)
            DeleteSelection(baseText, newCursor, initialCursorPos);

        SableString newText = baseText.substr(0, newCursor);
        for (unsigned int c : ctx.typedCharBuffer)
            newText.push_back((char32_t)c);

        newText = newText + baseText.substr(newCursor);

        setTextVal(newText);
        setCursorPos(newCursor + ctx.typedCharBuffer.size());
        setInitialCursorPos(-1);
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_BACKSPACE))
    {
        if (sectionHighlighted)
        {
            SableString newText = textVal;
            int newCursor = cursorPos;
            int init = initialCursorPos;

            DeleteSelection(newText, newCursor, init);

            setTextVal(newText);
            setCursorPos(newCursor);
            setInitialCursorPos(-1);
        }
        else if (ctrlDown)
        {
            int delTo = GetNextWordPos(textVal, cursorPos, -1);
			SableString newText = textVal.substr(0, delTo) + textVal.substr(cursorPos);
			setTextVal(newText);
			setCursorPos(delTo);
        }
        else if (cursorPos > 0)
        {
            SableString newText = textVal.substr(0, cursorPos - 1) + textVal.substr(cursorPos);
            setTextVal(newText);
            setCursorPos(cursorPos - 1);
        }
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_DELETE))
    {
        if (sectionHighlighted)
        {
            SableString newText = textVal;
            int newCursor = cursorPos;
            int init = initialCursorPos;

            DeleteSelection(newText, newCursor, init);

            setTextVal(newText);
            setCursorPos(newCursor);
            setInitialCursorPos(-1);
        }
        else if (ctrlDown)
        {
            int delTo = GetNextWordPos(textVal, cursorPos, 1);
            SableString newText = textVal.substr(0, cursorPos) + textVal.substr(delTo);
            setTextVal(newText);
        }
        else if (cursorPos < textVal.size())
        {
            SableString newText = textVal.substr(0, cursorPos) + textVal.substr(cursorPos + 1);
            setTextVal(newText);
        }
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_ENTER))
    {
        SableString baseText = textVal;
        int newCursor = cursorPos;

        if (sectionHighlighted)
            DeleteSelection(baseText, newCursor, initialCursorPos);

        SableString newText = baseText.substr(0, newCursor);
        newText.push_back(U'\n');
        newText = newText + baseText.substr(newCursor);

        setTextVal(newText);
        setCursorPos(newCursor + 1);
        setInitialCursorPos(-1);
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_LEFT))
    {
        if (sectionHighlighted)
            setCursorPos(std::min(cursorPos, initialCursorPos));
        
        if (ctrlDown)
            setCursorPos(GetNextWordPos(textVal, cursorPos, -1));
        else if (cursorPos > 0)
                setCursorPos(cursorPos - 1);

        if (!shiftDown)
            setInitialCursorPos(-1);
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_RIGHT))
    {
        if (sectionHighlighted)
			setCursorPos(std::max(cursorPos, initialCursorPos));

        if (ctrlDown)
            setCursorPos(GetNextWordPos(textVal, cursorPos, +1));
        else if (cursorPos < textVal.size())
                setCursorPos(cursorPos + 1);

        if (!shiftDown)
			setInitialCursorPos(-1);
    }
}

void SableUI::TextField::OnPostLayoutUpdate(const UIEventContext& ctx)
{
    if (!isFocused || !m_window) return;

    UseCustomLayoutContext(queue, m_window, m_window->GetSurface())
    {
        Element* text = GetElementById("TextFieldText");
        if (!text) return;

        auto cursorInfo = QueryCursorPosition(
            textVal,
            cursorPos,
            text->rect.w,
            text->fontSize,
            text->lineHeight,
            text->textJustification
        );

        Rect cursorRect = {
            text->rect.x + cursorInfo.x,
            text->rect.y + cursorInfo.y,
            1,
            cursorInfo.lineHeight
        };

        queue->AddRect(cursorRect, Colour(220, 220, 220));

        if (initialCursorPos >= 0 && initialCursorPos != cursorPos)
        {
			auto initialCursorInfo = QueryCursorPosition(
				textVal,
				initialCursorPos,
				text->rect.w,
				text->fontSize,
				text->lineHeight,
				text->textJustification
			);

            if (cursorInfo.lineIndex == initialCursorInfo.lineIndex)
            {
                Rect highlightRect = {
                    text->rect.x + std::min(initialCursorInfo.x, cursorInfo.x),
                    text->rect.y + cursorInfo.y,
                    std::abs(initialCursorInfo.x - cursorInfo.x),
                    cursorInfo.lineHeight
                };

                queue->AddRect(highlightRect, Colour(100, 180, 255, 120));
                return;
            }
        }
    }
}