//
// Created by pszemek on 27.05.2025.
//

#ifndef CONSOLECOLORS_H
#define CONSOLECOLORS_H

#include <Windows.h>

// Foreground colors
constexpr WORD FG_BLACK   = 0;
constexpr WORD FG_RED     = FOREGROUND_RED;
constexpr WORD FG_GREEN   = FOREGROUND_GREEN;
constexpr WORD FG_BLUE    = FOREGROUND_BLUE;
constexpr WORD FG_CYAN    = FOREGROUND_BLUE | FOREGROUND_GREEN;
constexpr WORD FG_YELLOW  = FOREGROUND_RED | FOREGROUND_GREEN;
constexpr WORD FG_MAGENTA = FOREGROUND_RED | FOREGROUND_GREEN;
constexpr WORD FG_GRAY         = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
constexpr WORD FG_WHITE        = FG_GRAY | FOREGROUND_INTENSITY;
constexpr WORD FG_BRIGHT_RED   = FG_RED   | FOREGROUND_INTENSITY;
constexpr WORD FG_BRIGHT_GREEN = FG_GREEN | FOREGROUND_INTENSITY;
constexpr WORD FG_BRIGHT_BLUE  = FG_BLUE  | FOREGROUND_INTENSITY;

// Background colors
constexpr WORD BG_RED    = BACKGROUND_RED;
constexpr WORD BG_GREEN  = BACKGROUND_GREEN;
constexpr WORD BG_BLUE   = BACKGROUND_BLUE;
constexpr WORD BG_YELLOW = BACKGROUND_RED | BACKGROUND_GREEN;
constexpr WORD BG_GRAY   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
constexpr WORD BG_WHITE  = BG_GRAY | BACKGROUND_INTENSITY;

#endif // CONSOLECOLORS_H
