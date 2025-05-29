//
// Created by pszemek on 27.05.2025.
//

#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

#include <Windows.h>
#include <vector>
#include <conio.h>

#include "Logger.h"

class ScreenBuffer {
public:
    short width;
    short height;
    std::vector<CHAR_INFO> buffer;

    ScreenBuffer() {
        hConsoleBuffer = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            CONSOLE_TEXTMODE_BUFFER,
            nullptr
        );

        if (hConsoleBuffer == INVALID_HANDLE_VALUE) {
            Logger::error("Failed to create screen buffer");
            exit(1);
        }

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            Logger::error("Failed to get screen info");
            exit(1);
        }

        width = static_cast<short>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        height = static_cast<short>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);

        resizeBuffer(width, height);
    }

    bool getInputChar(wchar_t &ch) {
        if (_kbhit()) {
            ch = _getwch();
            return true;
        }
        return false;
    }

    void activate() const {
        if (!SetConsoleActiveScreenBuffer(hConsoleBuffer)) {
            Logger::error("Failed to activate screen buffer");
        }
    }

    bool updateSizeIfChanged() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(this->hConsoleBuffer, &csbi)) {
            const short newWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            const short newHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

            if (newWidth != width || newHeight != height) {
                Logger::info("Screen resized: ", newWidth, "x", newHeight);
                width = newWidth;
                height = newHeight;
                resizeBuffer(newWidth, newHeight);

                return true;
            }
        } else {
            Logger::error("GetConsoleScreenBufferInfo failed");
        }

        return false;
    }

    void clear() {
        for (int i = 0; i < width * height; ++i) {
            buffer[i].Char.UnicodeChar = L' ';
            buffer[i].Attributes = 0;
        }
    }

    void render() const {
        SMALL_RECT rect = {0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        const COORD bufferSize = {width, height};
        constexpr COORD bufferCoord = {0, 0};

        WriteConsoleOutputW(
            hConsoleBuffer,
            buffer.data(),
            bufferSize,
            bufferCoord,
            &rect
        );
    }

private:
    HANDLE hConsoleBuffer;

    void resizeBuffer(const short newWidth, const short newHeight) {
        width = newWidth;
        height = newHeight;
        buffer.resize(width * height);
    }
};

#endif // SCREENBUFFER_H