//
// Created by pszemek on 28.05.2025.
//

#ifndef FOUNDATIONPILE_H
#define FOUNDATIONPILE_H

#include <vector>
#include "Card.h"
#include "ScreenBuffer.h"
#include "Renderable.h"

class FoundationPile : public Renderable {
public:
    FoundationPile() : Renderable(7, 7) {}

    void push(const Card& card) {
        cards.push_back(card);
    }

    void pop() {
        if (!cards.empty()) cards.pop_back();
    }

    const Card& peek() const {
        if (cards.empty()) throw std::out_of_range("FoundationPile empty");
        return cards.back();
    }

    bool empty() const {
        return cards.empty();
    }

    size_t size() const {
        return cards.size();
    }

    void reset() {
        cards.clear();
    }

    void render(ScreenBuffer& screen) {
        if (cards.empty()) {
            drawEmptyPlaceholder(screen);
        } else {
            Card& card = cards.back();
            card.setPos(posX, posY);
            card.render(screen);
        }
    }

private:
    std::vector<Card> cards;

    void drawEmptyPlaceholder(ScreenBuffer& screen) const {
        std::array<std::wstring, 7> placeholder = {
            L"+-----+",
            L"|     |",
            L"|     |",
            L"|     |",
            L"|     |",
            L"|     |",
            L"+-----+"
        };
        for (int i = 0; i < 7; i++) {
            drawText(screen, 0, i, placeholder[i], FG_WHITE | BG_GREEN);
        }
    }
};

#endif // FOUNDATIONPILE_H

