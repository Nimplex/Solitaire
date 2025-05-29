//
// Created by pszemek on 27.05.2025.
//

#ifndef CARD_H
#define CARD_H

#include <string>
#include <format>
#include <array>

#include "ScreenBuffer.h"
#include "Renderable.h"

enum class Suit {
    Hearts = 0, 
    Diamonds,
    Clubs,
    Spades,
    None
};

enum class Rank {
    Ace = 1,    // A = 1
    Two = 2,    // 2 = 2
    Three = 3,  // 3 = 3
    Four = 4,   // 4 = 4
    Five = 5,   // 5 = 5
    Six = 6,    // 6 = 6
    Seven = 7,  // 7 = 7
    Eight = 8,  // 8 = 8
    Nine = 9,   // 9 = 9
    Ten = 10,   // 10 = 10
    Jack = 11,  // J = 11
    Queen = 12, // Q = 12
    King = 13   // K = 13
};

inline bool isRedSuit(const Suit suit) {
    return suit == Suit::Hearts || suit == Suit::Diamonds;
}

inline wchar_t suitToWChar(const Suit suit) {
    switch (suit) {
        case Suit::Hearts:   return L'\u2665'; // ♥
        case Suit::Diamonds: return L'\u2666'; // ♦
        case Suit::Clubs:    return L'\u2663'; // ♣
        case Suit::Spades:   return L'\u2660'; // ♠
        case Suit::None:     return L' ';
        default:             return L'?';
    }
}

inline wchar_t rankToWChar(const Rank rank) {
    switch (rank) {
        case Rank::Jack:  return L'J';
        case Rank::Queen: return L'Q';
        case Rank::King:  return L'K';
        case Rank::Ace:   return L'A';
        case Rank::Ten:   return L'T';
        default:          return L'0' + static_cast<int>(rank);
    }
}

class Card : public Renderable {
public:
    Suit suit;
    Rank rank;
    bool isFaceUp;

    Card(const Suit s, const Rank r)
        : Renderable(7, 7), suit(s), rank(r), isFaceUp(false) {}

    void setBorder(const bool active) {
        borderActive = active;
    }

    void render(ScreenBuffer& screen, const bool small = false) const {
        clear(screen);

        if (!isFaceUp) {
            const std::array<std::wstring, 7> backPattern = {
                L"+-----+",
                L"|░░░░░|",
                L"|░░░░░|",
                L"|░░░░░|",
                L"|░░░░░|",
                L"|░░░░░|",
                L"+-----+"
            };
            for (int i = 0; i < 7; ++i) {
                drawText(screen, 0, i, backPattern[i], FG_WHITE | BG_BLUE);
            }
        } else {
            const wchar_t rankChar = rankToWChar(rank);
            const wchar_t suitChar = suitToWChar(suit);

            const WORD color = borderActive ? FG_BLACK | BG_YELLOW : (isRedSuit(suit) ? FG_RED : FG_BLACK) | BG_WHITE;

            const std::array<std::wstring, 7> lines = {
                L"+-----+",
                std::format(L"|{}{}   |", suitChar, small ? rankChar : L' '),
                L"|     |",
                std::format(L"|  {}  |", rankChar),
                L"|     |",
                std::format(L"|    {}|", suitChar),
                L"+-----+"
            };

            for (int i = 0; i < 7; ++i) {
                drawText(screen, 0, i, lines[i], color);
            }
        }
    }

private:
    bool borderActive = false;
};

#endif // CARD_H