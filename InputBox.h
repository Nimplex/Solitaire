//
// Created by pszemek on 28.05.2025.
//

#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <functional>
#include <string>
#include <algorithm>

#include "Renderable.h"
#include "Input.h"

class InputBox : public Renderable {
public:
    std::wstring buffer;
    std::function<void(const std::wstring&)> onEnter;

    InputBox(const int inputWidth, const std::wstring& label, const std::wstring& placeholder)
        : Renderable(static_cast<int>(label.size()) + inputWidth + 2, 1),
          label(label), placeholder(placeholder), prefix(label + L": "),
          inputWidth(inputWidth) {}

    void reset() {
        buffer.clear();
        cursorPos = 0;
    }

    void setActive(const bool isActive) {
        active = isActive;
    }

    void handleInput(const KeyEvent& event) {
        if (!active) return;

        switch (event.key) {
            case InputKey::LeftArrow:  moveCursorLeft(); break;
            case InputKey::RightArrow: moveCursorRight(); break;
            case InputKey::Character:  insertChar(event.ch); break;
            case InputKey::Backspace:  removeChar(); break;
            case InputKey::Enter:
                if (onEnter) {
                    onEnter(buffer);
                    reset();
                }
                break;
            default: break;
        }
    }

    void render(ScreenBuffer& screen) const {
        clear(screen);

        const WORD color = active ? (BG_BLUE | FG_WHITE) : (BG_GRAY | FG_BLACK);

        const bool showingPlaceholder = buffer.empty();
        const std::wstring& contentRef = showingPlaceholder ? placeholder : buffer;
        const std::wstring visibleContent = getVisibleContent(contentRef, showingPlaceholder);

        drawText(screen, 0, 0, prefix.c_str(), color);
        drawText(screen, static_cast<int>(prefix.size()), 0, visibleContent.c_str(),
                 color | (showingPlaceholder ? FG_GRAY : FG_WHITE));
    }

private:
    // Appearance
    std::wstring label;
    std::wstring placeholder;
    std::wstring prefix;
    int inputWidth;

    // State
    int cursorPos = 0;
    bool active = false;

    std::wstring getVisibleContent(const std::wstring& content, bool isPlaceholder) const {
        int start = 0;
        bool isScrolled = false;

        if (!isPlaceholder && cursorPos > inputWidth - 1) {
            start = cursorPos - (inputWidth - 1);
            isScrolled = true;
        }

        const int visibleLength = std::min(inputWidth - (isScrolled ? 1 : 0), static_cast<int>(content.size()) - start);
        std::wstring visible = content.substr(start, visibleLength);

        // Insert cursor
        if (active && !isPlaceholder) {
            int cursorInWindow = cursorPos - start;
            if (cursorInWindow >= 0 && cursorInWindow <= static_cast<int>(visible.size())) {
                visible.insert(cursorInWindow, 1, L'|');
            } else {
                visible += L"|";
            }
        }

        if (static_cast<int>(visible.size()) > (inputWidth - (isScrolled ? 1 : 0))) {
            visible = visible.substr(0, inputWidth - (isScrolled ? 1 : 0));
        }

        if (isScrolled) {
            visible = L"<" + visible;
        }

        if (static_cast<int>(visible.size()) < inputWidth) {
            visible += std::wstring(inputWidth - visible.size(), L'_');
        }

        return visible;
    }

    void moveCursorLeft() {
        if (cursorPos > 0) cursorPos--;
    }

    void moveCursorRight() {
        if (cursorPos < static_cast<int>(buffer.size())) cursorPos++;
    }

    void insertChar(wchar_t c) {
        buffer.insert(buffer.begin() + cursorPos, c);
        cursorPos++;
    }

    void removeChar() {
        if (cursorPos > 0 && !buffer.empty()) {
            buffer.erase(buffer.begin() + (--cursorPos));
        }
    }
};

#endif // INPUTBOX_H
