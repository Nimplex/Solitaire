#include "InputBox.h"
#include "SolitaireGame.h"

[[noreturn]] int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    bool renderGame = false;

    ScreenBuffer menuBuffer;
    menuBuffer.clear();
    menuBuffer.activate();

    ScreenBuffer gameBuffer;
    SolitaireGame game(gameBuffer.width, gameBuffer.height);

    InputBox input(20, L"Wprowadź swoje imie", L"np. monika");

    input.onEnter = [&renderGame, &gameBuffer, &input](const std::wstring& text) {
        input.setActive(false);
        renderGame = true;
        gameBuffer.activate();
    };

    while (true) {
        if (renderGame) {
            if (gameBuffer.updateSizeIfChanged()) {
                game.updateSize(gameBuffer);
                gameBuffer.clear();
            }

            KeyEvent kEvent = getInput();
            game.handleInput(kEvent);

            game.render(gameBuffer);
            gameBuffer.render();
        } else {
            input.setPos(menuBuffer.width / 2 - input.width / 2, menuBuffer.height / 2);
            input.render(menuBuffer);
            input.handleInput(getInput());
            menuBuffer.render();
        }
    }
}
