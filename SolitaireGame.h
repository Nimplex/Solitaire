#ifndef SOLITAIREGAME_H
#define SOLITAIREGAME_H

#include <vector>
#include <random>
#include <algorithm>

#include "Card.h"
#include "CardStash.h"
#include "ConsoleColors.h"
#include "ScreenBuffer.h"
#include "Renderable.h"
#include "TableauPile.h"
#include "FoundationPile.h"
#include "Input.h"

struct Selection {
    enum class Type {
        None, Stock, Waste, Foundation, Tableau
    } type = Type::None;

    int index = -1;   // e.g. which pile (0-6 for tableau, 0-3 for foundation, etc.)
    int cardIndex = -1; // for stacks like tableau - which card to start from

    bool isValid() const { return type != Type::None && index >= 0; }

    void clear() {
        type = Type::None;
        index = -1;
        cardIndex = -1;
    }
};

enum class MoveState {
    SelectingSource,
    SelectingCard,     // For tableau piles - selecting which card to start from
    SelectingDestination,
};

class SolitaireGame : public Renderable {
public:
    SolitaireGame(const int width, const int height)
        : Renderable(width, height), stock(), waste(),
          foundations(4), tableau(7) {
        setup();
    }

    void setup() {
        createDeck();
        shuffleDeck();
        dealToTableau();

        for (Card& card : allCards) {
            card.isFaceUp = false;
            stock.push(card);
        }

        allCards.clear();
    }

    void updateSize(ScreenBuffer& screen) {
        setSize(screen.width, screen.height);
        clear(screen);
    }

    void render(ScreenBuffer& screen) {
        clear(screen, BG_GREEN | FG_WHITE);

        waste.clear(screen, BG_GREEN);
        stock.setPos(2 - stock.renderBorder(), 2 - stock.renderBorder());
        stock.render(screen);
        drawText(screen, 4, 9, "[Q]", getSelectionColor(Selection::Type::Stock, 0));

        waste.clear(screen, BG_GREEN);
        waste.setPos(12 - waste.renderBorder(), 2 - waste.renderBorder());
        waste.render(screen);
        drawText(screen, 14, 9, "[W]", getSelectionColor(Selection::Type::Waste, 0));

        for (int i = 0; i < 4; i++) {
            const int fx = 32 + i * 10;
            const std::string labels[4] = { "[E]", "[R]", "[T]", "[Y]" };
            drawText(screen, fx + 2, 9, labels[i], getSelectionColor(Selection::Type::Foundation, i));

            foundations[i].setPos(fx, 2);
            foundations[i].render(screen);
        }

        for (int i = 0; i < 7; i++) {
            const int px = 2 + i * 10;
            std::string label = "[" + std::to_string(i + 1) + "]";
            drawText(screen, px + 2, 11, label, getSelectionColor(Selection::Type::Tableau, i));

            tableau[i].setPos(px, 12);
            tableau[i].setSelected(sourceSelection.type == Selection::Type::Tableau && sourceSelection.index == i, sourceSelection.cardIndex);
            tableau[i].render(screen);
        }

        renderStateInfo(screen);
    }

    void handleInput(const KeyEvent& input) {
        if (input.key == InputKey::Character) {
            if (std::toupper(input.ch) == 'Q' &&
                (moveState == MoveState::SelectingCard ||
                moveState == MoveState::SelectingDestination)) {
                moveState = MoveState::SelectingSource;
                sourceSelection.clear();
                destinationSelection.clear();
            } else if (moveState == MoveState::SelectingSource) {
                handleSourceSelection(input.ch);
            } else if (moveState == MoveState::SelectingDestination) {
                handleDestinationSelection(input.ch);
            }
        } else if (input.key == InputKey::LeftArrow || input.key == InputKey::RightArrow) {
            if (moveState == MoveState::SelectingCard) {
                handleCardSelection(input.key);
            }
        } else if (input.key == InputKey::Enter) {
            if (moveState == MoveState::SelectingCard) {
                confirmCardSelection();
            }
        }
    }

private:
    std::vector<Card> allCards;
    CardStash stock;
    CardStash waste;
    std::vector<FoundationPile> foundations;
    std::vector<TableauPile> tableau;
    MoveState moveState = MoveState::SelectingSource;
    Selection sourceSelection;
    Selection destinationSelection;

    void createDeck() {
        allCards.clear();
        for (int s = 0; s < 4; ++s) {
            for (int r = 2; r <= 14; ++r) {
                allCards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
            }
        }
    }

    void shuffleDeck() {
        std::random_device rd;
        std::mt19937 g(rd());
        std::ranges::shuffle(allCards, g);
    }

    void dealToTableau() {
        int index = 0;
        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j <= i; ++j) {
                Card card = allCards[index++];
                card.isFaceUp = (j == i);
                tableau[i].push(card);
            }
        }
        allCards.erase(allCards.begin(), allCards.begin() + index);
    }

    WORD getSelectionColor(const Selection::Type type, const int index) const {
        if (sourceSelection.type == type && sourceSelection.index == index) {
            return FG_BLACK | BG_YELLOW;
        }
        return FG_WHITE | BG_BLUE;
    }

    void renderStateInfo(ScreenBuffer& screen) const {
        std::string stateText;
        switch (moveState) {
            case MoveState::SelectingSource:
                stateText = "Select source pile (Q/W/E/R/T/Y/1-7)";
                break;
            case MoveState::SelectingCard:
                stateText = "Use arrow keys to select card, press Enter to confirm or Q to cancel move";
                break;
            case MoveState::SelectingDestination:
                stateText = "Select destination pile (Q/W/E/R/T/Y/1-7) or press Q to cancel move";
                break;
        }
        drawText(screen, 1, 0, stateText, FG_WHITE | BG_YELLOW);
    }

    void handleSourceSelection(const char ch) {
        Selection newSelection;

        switch (toupper(ch)) {
            case 'Q':
                newSelection = { Selection::Type::Stock, 0 };
                break;
            case 'W':
                newSelection = { Selection::Type::Waste, 0 };
                break;
            case 'E':
                newSelection = { Selection::Type::Foundation, 0 };
                break;
            case 'R':
                newSelection = { Selection::Type::Foundation, 1 };
                break;
            case 'T':
                newSelection = { Selection::Type::Foundation, 2 };
                break;
            case 'Y':
                newSelection = { Selection::Type::Foundation, 3 };
                break;
            case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                newSelection = { Selection::Type::Tableau, ch - '1' };
                break;
            default:
                return;
        }

        if (isValidSource(newSelection)) {
            sourceSelection = newSelection;

            if (newSelection.type == Selection::Type::Stock) {
                tryMove(sourceSelection, { Selection::Type::Waste, 0 });
                sourceSelection.clear();
                moveState = MoveState::SelectingSource;
            } else {
                // If it's a tableau pile with multiple cards, let user select which card
                sourceSelection.cardIndex = tableau[newSelection.index].size() - 1;
                if (newSelection.type == Selection::Type::Tableau
                    && tableau[newSelection.index].countFaceUp() > 1) {
                    moveState = MoveState::SelectingCard;
                } else {
                    moveState = MoveState::SelectingDestination;
                }
            }
        }
    }

    void handleCardSelection(const InputKey key) {
        if (sourceSelection.type != Selection::Type::Tableau) return;

        const int pileIndex = sourceSelection.index;
        const int maxCards = tableau[pileIndex].size();

        if (key == InputKey::LeftArrow && sourceSelection.cardIndex > 0) {
            sourceSelection.cardIndex--;
        } else if (key == InputKey::RightArrow && sourceSelection.cardIndex < maxCards - 1) {
            sourceSelection.cardIndex++;
        }

        // Ensure we can only select from face-up cards
        while (sourceSelection.cardIndex < maxCards - 1 &&
               !tableau[pileIndex].get(sourceSelection.cardIndex).isFaceUp) {
            sourceSelection.cardIndex++;
        }
    }

    void confirmCardSelection() {
        moveState = MoveState::SelectingDestination;
    }

    void handleDestinationSelection(const char ch) {
        Selection newSelection;

        switch (toupper(ch)) {
            case 'Q':
                newSelection = { Selection::Type::Stock, 0 };
                break;
            case 'E':
                newSelection = { Selection::Type::Foundation, 0 };
                break;
            case 'R':
                newSelection = { Selection::Type::Foundation, 1 };
                break;
            case 'T':
                newSelection = { Selection::Type::Foundation, 2 };
                break;
            case 'Y':
                newSelection = { Selection::Type::Foundation, 3 };
                break;
            case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                newSelection = { Selection::Type::Tableau, ch - '1' };
                break;
            default:
                return;
        }

        destinationSelection = newSelection;

        if (tryMove(sourceSelection, destinationSelection)) {
            // Move successful
        }

        // Reset selection state
        sourceSelection.clear();
        destinationSelection.clear();
        moveState = MoveState::SelectingSource;
    }

    bool isValidSource(const Selection& selection) const {
        switch (selection.type) {
            case Selection::Type::Stock:
                return true;
            case Selection::Type::Waste:
                return !waste.empty();
            case Selection::Type::Foundation:
                return !foundations[selection.index].empty();
            case Selection::Type::Tableau:
                return !tableau[selection.index].empty();
            default:
                return false;
        }
    }

    bool tryMove(const Selection& source, const Selection& dest) {
        // Handle stock to waste (draw cards)
        if (source.type == Selection::Type::Stock && dest.type == Selection::Type::Waste) {
            return drawFromStock();
        }

        std::vector<Card> cardsToMove;
        if (!getCardsToMove(source, cardsToMove)) {
            return false;
        }

        if (!isValidMove(cardsToMove, dest)) {
            return false;
        }

        removeCardsFromSource(source);
        addCardsToDestination(cardsToMove, dest);
        flipTopCardIfNeeded(source);

        return true;
    }

    bool drawFromStock() {
        if (stock.empty()) {
            // Recycle waste to stock
            while (!waste.empty()) {
                Card card = waste.peek();
                card.isFaceUp = false;
                waste.pop();
                stock.push(card);
            }
        } else {
            // Stock to waste
            Card card = stock.peek();
            card.isFaceUp = true;
            stock.pop();
            waste.push(card);
        }

        return true;
    }

    bool getCardsToMove(const Selection& source, std::vector<Card>& cardsToMove) const {
        cardsToMove.clear();

        switch (source.type) {
            case Selection::Type::Stock:
                if (!stock.empty()) {
                    cardsToMove.push_back(stock.peek());
                }
                break;
            case Selection::Type::Waste:
                if (!waste.empty()) {
                    cardsToMove.push_back(waste.peek());
                }
                break;
            case Selection::Type::Foundation:
                if (!foundations[source.index].empty()) {
                    cardsToMove.push_back(foundations[source.index].peek());
                }
                break;
            case Selection::Type::Tableau:
                {
                    const TableauPile& pile = tableau[source.index];
                    for (int i = source.cardIndex; i < pile.size(); i++) {
                        cardsToMove.push_back(pile.get(i));
                    }
                }
                break;
            default:
                return false;
        }

        return !cardsToMove.empty();
    }

    bool isValidMove(const std::vector<Card>& cards, const Selection& dest) const {
        if (cards.empty()) return false;

        const Card& bottomCard = cards[0];

        if (dest.type == Selection::Type::Foundation && cards.size() > 1)
            return false;

        switch (dest.type) {
            case Selection::Type::Foundation:
                return isValidFoundationMove(bottomCard, dest.index);
            case Selection::Type::Tableau:
                return isValidTableauMove(cards, dest.index);
            default:
                return false;
        }
    }

    bool isValidFoundationMove(const Card& card, const int foundationIndex) const {
        const auto& foundation = foundations[foundationIndex];

        if (foundation.empty()) {
            return card.rank == Rank::Ace;
        }

        const Card& topCard = foundation.peek();

        if (card.suit != topCard.suit) {
            return false;
        }

        if (topCard.rank == Rank::Ace && card.rank == Rank::Two) {
            return true;
        }

        return static_cast<int>(card.rank) == static_cast<int>(topCard.rank) + 1;
    }

    bool isValidTableauMove(const std::vector<Card>& cards, int tableauIndex) const {
        const Card& bottomCard = cards[0];
        const TableauPile& targetPile = tableau[tableauIndex];

        if (targetPile.empty()) {
            return bottomCard.rank == Rank::King;
        }

        const Card& topCard = targetPile.get(targetPile.size() - 1);
        const bool isBottomRed = (bottomCard.suit == Suit::Hearts || bottomCard.suit == Suit::Diamonds);
        const bool isTopRed = (topCard.suit == Suit::Hearts || topCard.suit == Suit::Diamonds);
        const bool oppositeColors = isBottomRed != isTopRed;
        const bool correctRank = static_cast<int>(bottomCard.rank) + 1 == static_cast<int>(topCard.rank);

        return oppositeColors && correctRank;
    }

    void removeCardsFromSource(const Selection& source) {
        switch (source.type) {
            case Selection::Type::Stock:
                stock.pop();
                break;
            case Selection::Type::Waste:
                waste.pop();
                break;
            case Selection::Type::Foundation:
                foundations[source.index].pop();
                break;
            case Selection::Type::Tableau:
                {
                    TableauPile& pile = tableau[source.index];
                    const int cardsToRemove = pile.size() - source.cardIndex;
                    for (int i = 0; i < cardsToRemove; i++) {
                        pile.pop();
                    }
                }
                break;
            default:
                break;
        }
    }

    void addCardsToDestination(const std::vector<Card>& cards, const Selection& dest) {
        switch (dest.type) {
            case Selection::Type::Foundation:
                foundations[dest.index].push(cards[0]);
                break;
            case Selection::Type::Tableau:
                for (const Card& card : cards) {
                    tableau[dest.index].push(card);
                }
                break;
            default:
                break;
        }
    }

    void flipTopCardIfNeeded(const Selection& source) {
        if (source.type == Selection::Type::Tableau) {
            TableauPile& pile = tableau[source.index];
            if (!pile.empty()) {
                pile.flipTopCard();
            }
        }
    }
};

#endif // SOLITAIREGAME_H