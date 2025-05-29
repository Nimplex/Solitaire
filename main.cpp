#include "InputBox.h"
#include "Selector.h"
#include "SolitaireGame.h"
#include "ScoreManager.h"

[[noreturn]] int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::wstring playerName;
    ScoreManager scoreManager("scores.txt"); // Save/load from file

    bool renderGame = false;
    bool preGameWon = false;
    Difficulty selectedDifficulty = Difficulty::Easy; // Store selected difficulty

    ScreenBuffer gameBuffer;
    SolitaireGame game(gameBuffer.width, gameBuffer.height);

    // --[MENU]---------------------------------------------------------------------------------------------------------
    ScreenBuffer menuBuffer;
    Renderable menu(menuBuffer.width, menuBuffer.height);

    menuBuffer.clear();
    menuBuffer.activate();
    menu.clear(menuBuffer, BG_GREEN);

    std::wstring text[] = {
        L"Instrukcja obsługi:                                                       ",
        L"- Wybierz stos spośród [QWERTY] i [1234567]                               ",
        L"- Jeżeli na stosie znajduje się wiele odkrytych kart,                     ",
        L"  wybierz jedną strzałkami [<-] / [->] a następnie zatwierdź [ENTER]      ",
        L"- Aby anulować ruch kliknij [Q]                                           ",
        L"- Okienko menu można zmieniać rozmiar - przeciągnij jego krawędzie myszką.",
        L"- Zmiana rozmiaru pozwala lepiej dopasować widok do Twoich potrzeb.       ",
        L"Ciesz się grą i powodzenia!                                               "
    };

    size_t linesCount = std::size(text);
    size_t maxLineLength = 0;
    for (size_t i = 0; i < linesCount; i++) {
        if (text[i].size() > maxLineLength) maxLineLength = text[i].size();
    }

    int startX = (menuBuffer.width - static_cast<int>(maxLineLength)) / 2;
    int startY = (menuBuffer.height / 2) - static_cast<int>(linesCount);

    for (size_t i = 0; i < linesCount; i++) {
        menu.drawText(menuBuffer, startX, startY + static_cast<int>(i), text[i].c_str(), FG_WHITE | 0);
    }

    std::vector<std::wstring> difficulties = {L"Łatwy", L"Ciężki"};
    Selector difficultySelector(difficulties, L"Poziom trudności");
    difficultySelector.setPos(menuBuffer.width / 2 - difficultySelector.width / 2, startY + static_cast<int>(linesCount) + 2);
    difficultySelector.setActive(true);

    InputBox input(20, L"Wprowadź swoje imie", L"np. monika");
    input.setPos(menuBuffer.width / 2 - input.width / 2, startY + static_cast<int>(linesCount) + 4);
    input.setActive(false);

    enum ActiveElement { DIFFICULTY, NAME_INPUT } activeElement = DIFFICULTY;

    difficultySelector.onSelect = [&game, &activeElement, &input, &difficultySelector, &selectedDifficulty](int index, const std::wstring& option) {
        selectedDifficulty = static_cast<Difficulty>(index);

        difficultySelector.setActive(false);
        input.setActive(true);
        game.setDifficulty(selectedDifficulty);
        activeElement = NAME_INPUT;
    };

    input.onEnter = [&game, &renderGame, &gameBuffer, &input, &playerName](const std::wstring& text) {
        playerName = text;
        input.setActive(false);
        renderGame = true;
        gameBuffer.activate();
        game.setup();
    };

    // --[WIN ]---------------------------------------------------------------------------------------------------------
    ScreenBuffer winBuffer;
    Renderable winScreen(winBuffer.width, winBuffer.height);

    winBuffer.clear();
    winScreen.clear(winBuffer, BG_GREEN);

    while (true) {
        if (renderGame) {
            if (gameBuffer.updateSizeIfChanged()) {
                game.updateSize(gameBuffer);
                gameBuffer.clear();
            }

            if (game.restartRequested) {
                game.setup(); // This will reset the game and clear the restart flag
                gameBuffer.clear();
            }

            if (game.isWin()) {
                renderGame = false;
                preGameWon = true;
                winBuffer.activate();
                std::string name(playerName.begin(), playerName.end());
                scoreManager.addScore(name, game.moves);
                continue;
            }

            KeyEvent kEvent = getInput();
            game.handleInput(kEvent);

            game.render(gameBuffer);
            gameBuffer.render();
        } else if (preGameWon) {
            if (gameBuffer.updateSizeIfChanged()) {
                winScreen.setSize(winBuffer.width, winBuffer.height);
                winBuffer.clear();
            }

            winScreen.clear(winBuffer, BG_GREEN);

            const auto& scores = scoreManager.getAllScores();
            constexpr int maxDisplay = 10;
            int count = std::min((int)scores.size(), maxDisplay);
            const int startY = winBuffer.height / 2 - count / 2;

            // Calculate the maximum line width for centering
            int maxLineWidth = 0;
            std::vector<std::wstring> lines;

            for (int i = 0; i < count; ++i) {
                const auto&[name, moves] = scores[i];
                std::wstring line = std::to_wstring(i + 1) + L". " +
                    std::wstring(name.begin(), name.end()) + L": " +
                    std::to_wstring(moves) + L" ruchów";
                lines.push_back(line);
                if (static_cast<int>(line.length()) > maxLineWidth) {
                    maxLineWidth = static_cast<int>(line.length());
                }
            }

            // Center the scoreboard
            int startX = (winBuffer.width - maxLineWidth) / 2;
            if (startX < 0) startX = 0;

            for (int i = 0; i < count; ++i) {
                winScreen.drawText(winBuffer, startX, startY + i, lines[i].c_str(), FG_WHITE | 0);
            }

            if (scores.size() > maxDisplay) {
                std::wstring more = L"<...pozostałe>";
                int moreX = (winBuffer.width - static_cast<int>(more.length())) / 2;
                if (moreX < 0) moreX = 0;
                winScreen.drawText(winBuffer, moreX, startY + count, more.c_str(), FG_WHITE | 0);
            }

            winBuffer.render();
        } else {
            KeyEvent event = getInput();

            if (activeElement == DIFFICULTY) {
                difficultySelector.handleInput(event);
            } else if (activeElement == NAME_INPUT) {
                input.handleInput(event);
            }

            difficultySelector.render(menuBuffer);
            input.render(menuBuffer);
            menuBuffer.render();
        }
    }
}