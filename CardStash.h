#ifndef CARDSTASH_H
#define CARDSTASH_H

#include <vector>

#include "Card.h"
#include "ConsoleColors.h"

class CardStash : public Renderable {
public:
    std::vector<Card> cards;

    CardStash() : Renderable(8, 8) {}

    bool renderBorder() const {
        return size() > 1;
    }

    void push(const Card& card) {
        cards.push_back(card);
    }

    void pop() {
        if (!cards.empty()) {
            return cards.pop_back();
        }
    }

    const Card& peek() const {
        if (cards.empty()) throw std::out_of_range("CardStash is empty");
        return cards.back();
    }

    bool empty() const {
        return cards.empty();
    }

    size_t size() const {
        return cards.size();
    }

    void render(ScreenBuffer& screen) const {
        if (empty()) return drawEmptyPlaceholder(screen);

        clear(screen, FG_WHITE | BG_GREEN);

        if (renderBorder()) {
            drawText(screen, 0, 0, L"+-----+", FG_WHITE | BG_GREEN);
            for (int i = 1; i < 6; i++) {
                drawText(screen, 0, i, L"|", FG_WHITE | BG_GREEN);
            }
            drawText(screen, 0, 6, L"+", FG_WHITE | BG_GREEN);
        }

        Card topCard = cards.back();
        topCard.setPos(posX + renderBorder(), posY + renderBorder());
        topCard.render(screen);
    }

private:
    void drawEmptyPlaceholder(ScreenBuffer& screen) const {
        const std::array<std::wstring, 7> placeholder = {
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

#endif // CARDSTASH_H
