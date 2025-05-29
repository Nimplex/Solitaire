//
// Created by pszemek on 28.05.2025.
//

#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "ConsoleColors.h"
#include "ScreenBuffer.h"

class Renderable {
public:
    int posX;
    int posY;
    int width;
    int height;

    Renderable(const int width, const int height): posX(0), posY(0), width(width), height(height) {};

    void setSize(const int width, const int height) {
        this->width = width;
        this->height = height;
    }

    void setPos(const int x, const int y) {
        posX = x;
        posY = y;
    }

    void clear(ScreenBuffer& screen, WORD color = FG_WHITE) const {
        for (int y1 = posY; y1 < posY + height && y1 < screen.height; y1++) {
            for (int x1 = posX; x1 < posX + width && x1 < screen.width; x1++) {
                screen.buffer[y1 * screen.width + x1].Char.UnicodeChar = L' ';
                screen.buffer[y1 * screen.width + x1].Attributes = color;
            }
        }
    }

    void drawText(ScreenBuffer& screen, const int x, const int y, const std::string& text, const WORD color) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if ((posY + y) >= screen.height) return;

        for (size_t i = 0; i < text.size(); i++) {
            if ((x + i) >= width || (posX + x + i) >= screen.width) continue;
            const int index = (posY + y) * screen.width + posX + x + i;
            screen.buffer[index].Char.AsciiChar = text[i];
            screen.buffer[index].Attributes = color;
        }
    }

    void drawText(ScreenBuffer& screen, const int x, const int y, const std::wstring& text, const WORD color) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if ((posY + y) >= screen.height) return;

        for (size_t i = 0; i < text.size(); i++) {
            if ((x + i) >= width || (posX + x + i) >= screen.width) continue;
            const int index = (posY + y) * screen.width + posX + x + i;
            screen.buffer[index].Char.UnicodeChar = text[i];
            screen.buffer[index].Attributes = color;
        }
    }
};

#endif //RENDERABLE_H
