//
// Created by pszemek on 29.05.2025.
//

#ifndef SELECTOR_H
#define SELECTOR_H

#include <functional>
#include <string>
#include <vector>

#include "Renderable.h"
#include "Input.h"

class Selector : public Renderable {
public:
    std::vector<std::wstring> options;
    std::function<void(int, const std::wstring&)> onSelect;

    Selector(const std::vector<std::wstring>& options, const std::wstring& label = L"")
        : Renderable(calculateWidth(options, label), 1),
          options(options), label(label), selectedIndex(0) {

        if (this->options.empty()) {
            this->options.push_back(L"No options available");
        }
    }

    void setOptions(const std::vector<std::wstring>& newOptions) {
        options = newOptions;
        if (options.empty()) {
            options.push_back(L"No options available");
        }

        selectedIndex = 0;
        setSize(calculateWidth(options, label), 1);
    }

    void setActive(const bool isActive) {
        active = isActive;
    }

    void setSelectedIndex(const int index) {
        if (index >= 0 && index < static_cast<int>(options.size())) {
            selectedIndex = index;
        }
    }

    int getSelectedIndex() const {
        return selectedIndex;
    }

    std::wstring getSelectedOption() const {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(options.size())) {
            return options[selectedIndex];
        }
        return L"";
    }

    void handleInput(const KeyEvent& event) {
        if (!active || options.empty()) return;

        switch (event.key) {
            case InputKey::LeftArrow:
                moveLeft();
                break;

            case InputKey::RightArrow:
                moveRight();
                break;

            case InputKey::Enter:
                if (onSelect && selectedIndex >= 0 && selectedIndex < static_cast<int>(options.size())) {
                    onSelect(selectedIndex, options[selectedIndex]);
                }
                break;

            default:
                break;
        }
    }

    void render(ScreenBuffer& screen) const {
        const WORD color = active ? (BG_BLUE | FG_WHITE) : (BG_GRAY | FG_BLACK);

        clear(screen, color);

        std::wstring displayText;

        if (!label.empty()) {
            displayText += label + L": ";
        }

        if (options.size() > 1) {
            displayText += L"< " + options[selectedIndex] + L" >";
        } else {
            displayText += options[selectedIndex];
        }

        drawText(screen, 0, 0, displayText, color);
    }

private:
    std::wstring label;
    int selectedIndex;
    bool active = false;

    static int calculateWidth(const std::vector<std::wstring>& options, const std::wstring& label) {
        int maxOptionWidth = 0;

        for (const auto& option : options) {
            const int optionWidth = static_cast<int>(option.size());
            if (optionWidth > maxOptionWidth) {
                maxOptionWidth = optionWidth;
            }
        }

        int totalWidth = maxOptionWidth;

        if (!label.empty()) {
            totalWidth += static_cast<int>(label.size()) + 2;
        }

        if (options.size() > 1) {
            totalWidth += 4;
        }

        return totalWidth;
    }

    void moveLeft() {
        if (selectedIndex > 0) {
            selectedIndex--;
        } else {
            selectedIndex = static_cast<int>(options.size()) - 1;
        }
    }

    void moveRight() {
        if (selectedIndex < static_cast<int>(options.size()) - 1) {
            selectedIndex++;
        } else {
            selectedIndex = 0;
        }
    }
};

#endif // SELECTOR_H