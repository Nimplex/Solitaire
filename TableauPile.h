#ifndef TABLEUPILE_H
#define TABLEUPILE_H

#include <vector>
#include <stdexcept>
#include "Card.h"
#include "ScreenBuffer.h"
#include "Renderable.h"

class TableauPile : public Renderable {
public:
    TableauPile() : Renderable(7, 20), selected(false), selectedCard(0) {}

    void setSelected(const bool isSelected, const int index) {
        selected = isSelected;
        selectedCard = index;
    }

    int countFaceUp() const {
        int count = 0;
        for (const Card& card : cards) {
            if (card.isFaceUp) {
                count++;
            }
        }
        return count;
    }

    void push(const Card& card) {
        cards.push_back(card);
    }

    void pop() {
        if (!cards.empty()) cards.pop_back();
    }

    size_t size() const {
        return cards.size();
    }

    bool empty() const {
        return cards.empty();
    }

    const Card& get(const size_t index) const {
        if (index >= cards.size()) throw std::out_of_range("TableauPile index out of range");
        return cards[index];
    }

    Card& get(const size_t index) {
        if (index >= cards.size()) throw std::out_of_range("TableauPile index out of range");
        return cards[index];
    }

    void reset() {
        cards.clear();
    }

    void flipTopCard() {
        if (!cards.empty()) {
            cards.back().isFaceUp = true;
        }
    }

    // Check if a sequence of cards from startIndex is valid (alternating colors, descending ranks)
    bool isValidSequence(const size_t startIndex) const {
        if (startIndex >= cards.size()) return false;

        for (size_t i = startIndex; i < cards.size() - 1; i++) {
            const Card& current = cards[i];
            const Card& next = cards[i + 1];

            // Check if cards are face up
            if (!current.isFaceUp || !next.isFaceUp) return false;

            // Check opposite colors: Hearts and Diamonds are red, Clubs and Spades are black
            const bool isCurrentRed = (current.suit == Suit::Hearts || current.suit == Suit::Diamonds);
            const bool isNextRed = (next.suit == Suit::Hearts || next.suit == Suit::Diamonds);
            const bool oppositeColors = isCurrentRed != isNextRed;

            // Check descending ranks (lower rank goes on higher rank)
            const bool correctRank = static_cast<int>(current.rank) + 1 == static_cast<int>(next.rank);

            if (!oppositeColors || !correctRank) return false;
        }

        return true;
    }

    void render(ScreenBuffer& screen) const {
        for (size_t i = 0; i < cards.size(); i++) {
            Card card = cards[i];
            card.setPos(posX, i * 2 + posY);
            card.setBorder(selected ? selectedCard == i : false);
            card.render(screen, i != cards.size() - 1);
        }

        if (cards.size() == 0) {
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
    }

private:
    std::vector<Card> cards;
    bool selected;
    int selectedCard;
};

#endif // TABLEUPILE_H