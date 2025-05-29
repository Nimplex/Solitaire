//
// Created by pszemek on 28.05.2025.
//

#ifndef SOLITAIREGAME_H
#define SOLITAIREGAME_H

#include <vector>
#include <random>
#include <algorithm>
#include <stack>

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
    SelectingCard,
    SelectingDestination,
};

enum class Difficulty {
    Easy = 0,
    Hard = 1
};

// Structure to store move information for undo functionality
struct Move {
    enum class Type {
        StockToWaste,
        WasteToStock,  // For recycling waste back to stock
        CardMove,      // Moving cards between piles
        FlipCard       // Flipping a card face up
    } type;

    Selection source;
    Selection destination;
    std::vector<Card> movedCards;
    bool wasCardFlipped = false;
    int sourceIndex = -1;  // For flip operations

    Move(const Type t) : type(t) {}
    Move(const Type t, const Selection src, const Selection dest, const std::vector<Card>& cards, const bool flipped = false)
        : type(t), source(src), destination(dest), movedCards(cards), wasCardFlipped(flipped) {}
};

class SolitaireGame : public Renderable {
public:
    int moves;
    bool restartRequested = false; // Flag to signal restart request

    SolitaireGame(const int width, const int height)
        : Renderable(width, height), stock(), waste(),
          foundations(4), tableau(7), difficulty(), moves(0), maxUndoMoves(3), duringSetup(true) {
    }

    void setDifficulty(const Difficulty difficulty) {
        this->difficulty = difficulty;
    }

    void setup() {
        duringSetup = true;
        stock.cards.clear();
        waste.cards.clear();
        restartRequested = false;

        for (auto& foundation : foundations) {
            foundation.reset();
        }

        for (auto& pile : tableau) {
            pile.reset();
        }

        createDeck();
        shuffleDeck();
        dealToTableau();

        // Move remaining cards to stock
        for (Card& card : allCards) {
            card.isFaceUp = false;
            stock.push(card);
        }

        moves = 0;

        // Clear move history
        while (!moveHistory.empty()) {
            moveHistory.pop();
        }

        // Reset selection state
        sourceSelection.clear();
        destinationSelection.clear();
        moveState = MoveState::SelectingSource;

        duringSetup = false;
    }

    void updateSize(ScreenBuffer& screen) {
        setSize(screen.width, screen.height);
        clear(screen);
    }

    void render(ScreenBuffer& screen) {
        clear(screen, BG_GREEN | FG_WHITE);

        // Render stock pile
        stock.clear(screen, BG_GREEN);
        stock.setPos(2 - stock.renderBorder(), 2 - stock.renderBorder());
        stock.render(screen);
        drawText(screen, 4, 9, "[Q]", getSelectionColor(Selection::Type::Stock, 0));

        // Render waste pile
        waste.clear(screen, BG_GREEN);
        waste.setPos(12 - waste.renderBorder(), 2 - waste.renderBorder());
        waste.render(screen);
        drawText(screen, 14, 9, "[W]", getSelectionColor(Selection::Type::Waste, 0));

        // Render foundation piles
        for (int i = 0; i < 4; i++) {
            const int fx = 32 + i * 10;
            const std::string labels[4] = { "[E]", "[R]", "[T]", "[Y]" };
            drawText(screen, fx + 2, 9, labels[i], getSelectionColor(Selection::Type::Foundation, i));

            foundations[i].setPos(fx, 2);
            foundations[i].render(screen);
        }

        // Render tableau piles
        for (int i = 0; i < 7; i++) {
            const int px = 2 + i * 10;
            std::string label = "[" + std::to_string(i + 1) + "]";
            drawText(screen, px + 2, 11, label, getSelectionColor(Selection::Type::Tableau, i));

            tableau[i].setPos(px, 12);
            tableau[i].setSelected(sourceSelection.type == Selection::Type::Tableau && sourceSelection.index == i, sourceSelection.cardIndex);
            tableau[i].render(screen);
        }

        renderStateInfo(screen);
        renderMoveCount(screen);
        renderUndoInfo(screen);
        renderRestartInfo(screen); // Add restart info
    }

    void handleInput(const KeyEvent& input) {
        if (input.key == InputKey::Character) {
            if (std::toupper(input.ch) == 'Q' &&
                (moveState == MoveState::SelectingCard ||
                moveState == MoveState::SelectingDestination)) {
                moveState = MoveState::SelectingSource;
                sourceSelection.clear();
                destinationSelection.clear();
            } else if (std::toupper(input.ch) == 'U') {
                // Undo move (limit to 3 moves back)
                undoLastMove();
            } else if (std::toupper(input.ch) == 'R') {
                // Restart game
                restartRequested = true;
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

    bool isWin() const {
        if (duringSetup) return false;

        // Check if all four foundation piles are complete (Ace to King = 13 cards each)
        for (const auto& foundation : foundations) {
            if (foundation.size() != 13) {
                return false;
            }

            // Verify the top card is a King
            if (!foundation.empty() && foundation.peek().rank != Rank::King) {
                return false;
            }
        }

        return true; // All foundations are complete
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
    Difficulty difficulty;
    std::stack<Move> moveHistory;
    const int maxUndoMoves; // Limit to 3 undo moves
    bool duringSetup;

    void createDeck() {
        allCards.clear();
        // Create standard 52-card deck: A, 2, 3, 4, 5, 6, 7, 8, 9, 10, J, Q, K for each suit
        for (int s = 0; s < 4; ++s) {
            for (int r = 1; r <= 13; ++r) { // Changed from r = 2 to r = 1 to include Ace
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
                card.isFaceUp = (j == i); // Only the last card in each column is face-up
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
        std::wstring stateText;
        switch (moveState) {
            case MoveState::SelectingSource:
                stateText = L"Wybierz stos [Q/W/E/R/T/Y/1-7] | Cofnij ruch [U] | Restart [R]";
                break;
            case MoveState::SelectingCard:
                stateText = L"Użyj strzałek aby wybrać karte, zatwierdź [Enter] lub odrzuć [Q]";
                break;
            case MoveState::SelectingDestination:
                stateText = L"Wybierz stos docelowy [Q/W/E/R/T/Y/1-7] lub anuluj [Q]";
                break;
        }
        drawText(screen, 1, 0, stateText, FG_WHITE | 0);
    }

    void renderMoveCount(ScreenBuffer& screen) const {
        const std::wstring moveText = L"Ruchy: " + std::to_wstring(moves);
        drawText(screen, width - static_cast<int>(moveText.length()) - 2, 0, moveText, FG_YELLOW | 0);
    }

    void renderUndoInfo(ScreenBuffer& screen) const {
        const int undoCount = std::min(static_cast<int>(moveHistory.size()), maxUndoMoves);
        const std::wstring undoText = L"Cofnij: " + std::to_wstring(undoCount) + L"/" + std::to_wstring(maxUndoMoves);
        drawText(screen, width - static_cast<int>(undoText.length()) - 2, 1, undoText, FG_CYAN | 0);
    }

    void renderRestartInfo(ScreenBuffer& screen) const {
        const std::wstring restartText = L"Restart [R]";
        drawText(screen, width - static_cast<int>(restartText.length()) - 2, 2, restartText, FG_MAGENTA | 0);
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
                if (newSelection.type == Selection::Type::Tableau) {
                    sourceSelection.cardIndex = tableau[newSelection.index].size() - 1;
                    if (tableau[newSelection.index].countFaceUp() > 1) {
                        moveState = MoveState::SelectingCard;
                    } else {
                        moveState = MoveState::SelectingDestination;
                    }
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
        // Handle stock to waste (draw cards based on difficulty)
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

        // Store move for undo functionality
        Move move(Move::Type::CardMove, source, dest, cardsToMove);

        // Check if we need to flip a card after this move
        if (source.type == Selection::Type::Tableau) {
            TableauPile& pile = tableau[source.index];
            int newTopIndex = source.cardIndex - 1;
            if (newTopIndex >= 0 && !pile.get(newTopIndex).isFaceUp) {
                move.wasCardFlipped = true;
                move.sourceIndex = newTopIndex;
            }
        }

        removeCardsFromSource(source);
        addCardsToDestination(cardsToMove, dest);
        flipTopCardIfNeeded(source);

        moves++;
        moveHistory.push(move);

        // Limit undo history to maxUndoMoves
        limitUndoHistory();

        return true;
    }

    bool drawFromStock() {
        if (stock.empty()) {
            // Reshuffle waste back to stock
            if (waste.empty()) return false;

            Move move(Move::Type::WasteToStock);
            std::vector<Card> wasteCards;

            // Collect all waste cards
            while (!waste.empty()) {
                Card card = waste.peek();
                card.isFaceUp = false;
                wasteCards.push_back(card);
                waste.pop();
            }

            // Shuffle the cards before putting them back to stock
            std::random_device rd;
            std::mt19937 g(rd());
            std::ranges::shuffle(wasteCards, g);

            // Add shuffled cards to stock
            for (const Card& card : wasteCards) {
                stock.push(card);
            }

            move.movedCards = wasteCards;
            moves++;
            moveHistory.push(move);
            limitUndoHistory();
        } else {
            // Draw cards from stock to waste based on difficulty
            if (difficulty == Difficulty::Easy) {
                // Easy: Draw 1 card
                drawCardsFromStock(1);
            } else {
                // Hard: Draw 3 cards (or remaining cards if less than 3)
                const int cardsToDraw = std::min(3, static_cast<int>(stock.size()));
                drawCardsFromStock(cardsToDraw);
            }
        }

        return true;
    }

    void drawCardsFromStock(const int numCards) {
        Move move(Move::Type::StockToWaste);

        for (int i = 0; i < numCards && !stock.empty(); i++) {
            Card card = stock.peek();
            card.isFaceUp = true;
            move.movedCards.push_back(card);
            stock.pop();
            waste.push(card);
        }

        moves++;
        moveHistory.push(move);
        limitUndoHistory();
    }

    void limitUndoHistory() {
        // Keep only the last maxUndoMoves moves
        std::stack<Move> tempStack;
        int keepCount = 0;

        while (!moveHistory.empty() && keepCount < maxUndoMoves) {
            tempStack.push(moveHistory.top());
            moveHistory.pop();
            keepCount++;
        }

        // Clear remaining history
        while (!moveHistory.empty()) {
            moveHistory.pop();
        }

        // Restore the moves we want to keep
        while (!tempStack.empty()) {
            moveHistory.push(tempStack.top());
            tempStack.pop();
        }
    }

    void undoLastMove() {
        if (moveHistory.empty()) return;

        const Move lastMove = moveHistory.top();
        moveHistory.pop();

        switch (lastMove.type) {
            case Move::Type::StockToWaste:
                // Move cards back from waste to stock
                for (int i = lastMove.movedCards.size() - 1; i >= 0; i--) {
                    if (!waste.empty()) {
                        Card card = waste.peek();
                        card.isFaceUp = false;
                        waste.pop();
                        stock.push(card);
                    }
                }
                break;

            case Move::Type::WasteToStock:
                // Move cards back from stock to waste (reverse the reshuffle)
                for (const Card& card : lastMove.movedCards) {
                    if (!stock.empty()) {
                        stock.pop();
                    }
                }
                // Add cards back to waste in original order
                for (int i = lastMove.movedCards.size() - 1; i >= 0; i--) {
                    Card card = lastMove.movedCards[i];
                    card.isFaceUp = true;
                    waste.push(card);
                }
                break;

            case Move::Type::CardMove:
                // Move cards back from destination to source
                undoCardMove(lastMove);
                break;

            default:
                break;
        }

        moves--;
        if (moves < 0) moves = 0;
    }

    void undoCardMove(const Move& move) {
        // Remove cards from destination
        switch (move.destination.type) {
            case Selection::Type::Foundation:
                for (size_t i = 0; i < move.movedCards.size(); i++) {
                    if (!foundations[move.destination.index].empty()) {
                        foundations[move.destination.index].pop();
                    }
                }
                break;
            case Selection::Type::Tableau:
                for (size_t i = 0; i < move.movedCards.size(); i++) {
                    if (!tableau[move.destination.index].empty()) {
                        tableau[move.destination.index].pop();
                    }
                }
                break;
            default:
                break;
        }

        // Add cards back to source
        switch (move.source.type) {
            case Selection::Type::Waste:
                for (const Card& card : move.movedCards) {
                    waste.push(card);
                }
                break;
            case Selection::Type::Foundation:
                for (const Card& card : move.movedCards) {
                    foundations[move.source.index].push(card);
                }
                break;
            case Selection::Type::Tableau:
                for (const Card& card : move.movedCards) {
                    tableau[move.source.index].push(card);
                }
                // If a card was flipped during the original move, flip it back
                if (move.wasCardFlipped && move.sourceIndex >= 0) {
                    if (move.sourceIndex < tableau[move.source.index].size()) {
                        tableau[move.source.index].get(move.sourceIndex).isFaceUp = false;
                    }
                }
                break;
            default:
                break;
        }
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
            return card.rank == Rank::Ace; // Foundation must start with Ace
        }

        const Card& topCard = foundation.peek();

        // Must be same suit
        if (card.suit != topCard.suit) {
            return false;
        }

        // Must be next rank in sequence (Ace=1, Two=2, ..., King=13)
        return static_cast<int>(card.rank) == static_cast<int>(topCard.rank) + 1;
    }

    bool isValidTableauMove(const std::vector<Card>& cards, const int tableauIndex) const {
        const Card& bottomCard = cards[0];
        const TableauPile& targetPile = tableau[tableauIndex];

        if (targetPile.empty()) {
            return bottomCard.rank == Rank::King; // Empty tableau pile must start with King
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