#include <SableUI/components/text_field.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/text.h>
#include <SableUI/styles/styles.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/utils.h>
#include <algorithm>
#include <cmath>

using namespace SableUI::Style;

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

static void DeleteSelection(SableString& text, int& cursorPos, const int& initialCursorPos)
{
    int selStart = std::min(cursorPos, initialCursorPos);
    int selEnd = std::max(cursorPos, initialCursorPos);

    selStart = std::max(0, selStart);
    selEnd = std::max(0, selEnd);

    text = text.substr(0, static_cast<size_t>(selStart)) +
        text.substr(static_cast<size_t>(selEnd));

    cursorPos = selStart;
}

SableUI::TextFieldComponent::TextFieldComponent()
{
    m_window = GetContext();
}

void SableUI::TextFieldComponent::Init(State<InputFieldData>& data,
    const ElementInfo& p_info,
    bool multiline)
{
    info = p_info;
    externalState = &data;
    m_multiline = multiline;
}

void SableUI::TextFieldComponent::ResetCursorBlink()
{
    cursorVisible.set(true);
}

void SableUI::TextFieldComponent::TriggerOnChange()
{
    if (externalState && externalState->get().onChange)
    {
        externalState->get().onChange();
    }
}

void SableUI::TextFieldComponent::Layout()
{
    if (!externalState) return;
    const InputFieldData& data = externalState->get();

    const Theme& t = GetTheme();
    if (!m_window)
    {
        Text("Component does not have m_window defined", textColour(t.red), mb(4));
        return;
    }

    Colour bgColour = info.appearance.bg != Colour{ 0, 0, 0, 0 } ? info.appearance.bg : t.surface0;

    Colour textCol = info.text.colour.has_value() ? info.text.colour.value() : t.text;

    Colour placeholderCol = t.subtext0;

    float borderRadius = info.appearance.radius > 0 ? info.appearance.radius : 4.0f;

    ElementInfo containerInfo = info;
    containerInfo.appearance.bg = bgColour;
    containerInfo.appearance.radius = borderRadius;

    if (containerInfo.layout.wType == RectType::Undef) containerInfo.layout.wType = RectType::Fill;
    if (containerInfo.layout.hType == RectType::Undef) containerInfo.layout.hType = RectType::FitContent;

    if (containerInfo.layout.pT == 0 && containerInfo.layout.pB == 0
        && containerInfo.layout.pL == 0 && containerInfo.layout.pR == 0)
    {
        containerInfo.layout.pT = containerInfo.layout.pB = 4;
        containerInfo.layout.pL = containerInfo.layout.pR = 4;
    }

    PackStylesToInfo(containerInfo, id("TextField"));

    if (DivScope d(containerInfo); true)
    {
        if (data.content.empty() && !data.isFocused)
        {
            Text(data.placeholder, textColour(placeholderCol), wrapText(m_multiline));
        }
        else
        {
            Text(data.content, id("TextFieldText"), textColour(textCol), wrapText(m_multiline));
        }
    }
}

void SableUI::TextFieldComponent::OnUpdate(const UIEventContext& ctx)
{
    if (!externalState) return;
    InputFieldData dataCopy = externalState->get();

    bool ctrlDown = (ctx.isKeyDown.test(SABLE_KEY_LEFT_CONTROL) || ctx.isKeyDown.test(SABLE_KEY_RIGHT_CONTROL));
    bool shiftDown = (ctx.isKeyDown.test(SABLE_KEY_LEFT_SHIFT) || ctx.isKeyDown.test(SABLE_KEY_RIGHT_SHIFT));
    bool sectionHighlighted = initialCursorPos >= 0 && initialCursorPos != cursorPos;
    bool change = false;

    if (ctx.keyPressedEvent.test(SABLE_KEY_LEFT_SHIFT) || ctx.keyPressedEvent.test(SABLE_KEY_RIGHT_SHIFT))
    {
        initialCursorPos.set(cursorPos);
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_ESCAPE))
    {
        if (initialCursorPos == -1)
        {
            dataCopy.isFocused = false;
            if (queueInitialised)
                queue.window->RemoveQueueReference(&queue);
        }

        initialCursorPos.set(-1);
    }

    // Handle mouse clicks for focus
    if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
    {
        Element* el = GetElementById("TextField");
        if (el)
        {
            bool clickedInside = RectBoundingBox(el->rect, ctx.mousePos);

            if (clickedInside)
            {
                if (!dataCopy.isFocused)
                {
                    dataCopy.isFocused = true;
                    ResetCursorBlink();
                    change = true;
                }
            }
            else
            {
                if (dataCopy.isFocused)
                {
                    dataCopy.isFocused = false;
                    if (queueInitialised)
                        queue.window->RemoveQueueReference(&queue);
                    change = true;
                }
            }
        }
    }

    if (!dataCopy.isFocused)
    {
        if (change)
            externalState->set(dataCopy);
        return;
    }

    if (ctx.typedCharBuffer.size() != 0)
    {
        SableString baseText = dataCopy.content;
        int newCursor = cursorPos;

        if (sectionHighlighted)
            DeleteSelection(baseText, newCursor, initialCursorPos.get());

        SableString newText = baseText.substr(0, newCursor);
        for (unsigned int c : ctx.typedCharBuffer)
        {
            if (!m_multiline && (c == '\n' || c == '\r'))
                continue;

            newText.push_back((char32_t)c);
        }

        newText = newText + baseText.substr(newCursor);

        dataCopy.content = newText;
        cursorPos.set(newCursor + ctx.typedCharBuffer.size());
        initialCursorPos.set(-1);
        change = true;
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_BACKSPACE))
    {
        if (sectionHighlighted)
        {
            SableString newText = dataCopy.content;
            int newCursor = cursorPos;
            int init = initialCursorPos;

            DeleteSelection(newText, newCursor, init);

            dataCopy.content = newText;
            cursorPos.set(newCursor);
            initialCursorPos.set(-1);
        }
        else if (ctrlDown)
        {
            int delTo = GetNextWordPos(dataCopy.content, cursorPos, -1);
            SableString newText = dataCopy.content.substr(0, delTo) + dataCopy.content.substr(cursorPos);
            dataCopy.content = newText;
            cursorPos.set(delTo);
        }
        else if (cursorPos > 0)
        {
            SableString newText = dataCopy.content.substr(0, cursorPos - 1) + dataCopy.content.substr(cursorPos);
            dataCopy.content = newText;
            cursorPos.set(cursorPos - 1);
        }
        change = true;
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_DELETE))
    {
        if (sectionHighlighted)
        {
            SableString newText = dataCopy.content;
            int newCursor = cursorPos;
            int init = initialCursorPos;

            DeleteSelection(newText, newCursor, init);

            dataCopy.content = newText;
            cursorPos.set(newCursor);
            initialCursorPos.set(-1);
        }
        else if (ctrlDown)
        {
            int delTo = GetNextWordPos(dataCopy.content, cursorPos, 1);
            SableString newText = dataCopy.content.substr(0, cursorPos) + dataCopy.content.substr(delTo);
            dataCopy.content = newText;
        }
        else if (cursorPos < dataCopy.content.size())
        {
            SableString newText = dataCopy.content.substr(0, cursorPos) + dataCopy.content.substr(cursorPos + 1);
            dataCopy.content = newText;
        }
        change = true;
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_ENTER))
    {
        if (m_multiline)
        {
            SableString baseText = dataCopy.content;
            int newCursor = cursorPos;

            if (sectionHighlighted)
                DeleteSelection(baseText, newCursor, initialCursorPos);

            SableString newText = baseText.substr(0, newCursor);
            newText.push_back(U'\n');
            newText = newText + baseText.substr(newCursor);

            dataCopy.content = newText;
            cursorPos.set(newCursor + 1);
            initialCursorPos.set(-1);
            change = true;
        }
        else
        {
            if (dataCopy.onSubmit)
            {
                dataCopy.onSubmit();
            }
        }
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_LEFT))
    {
        if (sectionHighlighted)
            cursorPos.set(std::min(cursorPos.get(), initialCursorPos.get()));

        if (ctrlDown)
            cursorPos.set(GetNextWordPos(dataCopy.content, cursorPos, -1));
        else if (cursorPos > 0)
            cursorPos.set(cursorPos - 1);

        if (!shiftDown)
            initialCursorPos.set(-1);

        change = true;
    }

    if (ctx.keyPressedEvent.test(SABLE_KEY_RIGHT))
    {
        if (sectionHighlighted)
            cursorPos.set(std::max(cursorPos.get(), initialCursorPos.get()));

        if (ctrlDown)
            cursorPos.set(GetNextWordPos(dataCopy.content, cursorPos, +1));
        else if (cursorPos < dataCopy.content.size())
            cursorPos.set(cursorPos + 1);

        if (!shiftDown)
            initialCursorPos.set(-1);

        change = true;
    }

    if (change)
    {
        ResetCursorBlink();
        TriggerOnChange();
    }

    externalState->set(dataCopy);
}

void SableUI::TextFieldComponent::OnUpdatePostLayout(const UIEventContext& ctx)
{
    if (!externalState || !externalState->get().isFocused || !m_window) return;

    if (!queueInitialised)
    {
        queue.window = m_window;
        queue.target = m_window->GetSurface();
        queueInitialised = true;
    }

    StartCustomLayoutScope(&queue);

    Element* text = GetElementById("TextFieldText");
    if (!text)
    {
        EndCustomLayoutScope(&queue);
        return;
    }

    auto cursorInfo = QueryCursorPosition(
        externalState->get().content,
        cursorPos,
        text->rect.w,
        text->info.text.fontSize,
        text->info.text.lineHeight,
        text->info.text.justification
    );

    if (cursorVisible.get())
    {
        Rect cursorRect = {
            text->rect.x + cursorInfo.x,
            text->rect.y + cursorInfo.y,
            1,
            cursorInfo.lineHeight
        };

        queue.AddRect(cursorRect, Colour(220, 220, 220));
    }

    if (initialCursorPos >= 0 && initialCursorPos != cursorPos)
    {
        auto initialCursorInfo = QueryCursorPosition(
            externalState->get().content,
            initialCursorPos,
            text->rect.w,
            text->info.text.fontSize,
            text->info.text.lineHeight,
            text->info.text.justification
        );

        if (cursorInfo.lineIndex == initialCursorInfo.lineIndex)
        {
            Rect highlightRect = {
                text->rect.x + std::min(initialCursorInfo.x, cursorInfo.x),
                text->rect.y + cursorInfo.y,
                std::abs(initialCursorInfo.x - cursorInfo.x),
                cursorInfo.lineHeight
            };

            queue.AddRect(highlightRect, Colour(80, 150, 255, 120));
        }
    }

    EndCustomLayoutScope(&queue);
}