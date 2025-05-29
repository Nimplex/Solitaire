//
// Created by pszemek on 28.05.2025.
//

#ifndef INPUT_H
#define INPUT_H
#include <conio.h>

enum class InputKey {
    None,
    Character,
    Enter,
    Backspace,
    LeftArrow,
    RightArrow
};

struct KeyEvent {
    InputKey key;
    char ch;
};

inline KeyEvent getInput() {
    if (!_kbhit()) return {InputKey::None, 0};

    const int first = _getch();

    if (first == 0 || first == 224) {
        int code = _getch();
        switch (code) {
            case 75: return {InputKey::LeftArrow, 0};
            case 77: return {InputKey::RightArrow, 0};
            default: return {InputKey::None, 0};
        }
    }

    if (first == '\r') return {InputKey::Enter, 0};
    if (first == '\b') return {InputKey::Backspace, 0};

    return {InputKey::Character, static_cast<char>(first)};
}

#endif //INPUT_H
