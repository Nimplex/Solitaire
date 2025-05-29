//
// Created by pszemek on 29.05.2025.
//

#ifndef TEXT_H
#define TEXT_H
#include "Renderable.h"

class Text : public Renderable {
public:
    Text(const std::string& message, const WORD color = FG_WHITE | BG_BLUE) : Renderable(static_cast<int>(message.length()), 1) {};

    render() {
        drawText()
    }
};

#endif //TEXT_H
