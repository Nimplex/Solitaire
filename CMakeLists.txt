cmake_minimum_required(VERSION 3.31)
project(Solitaire)

set(CMAKE_CXX_STANDARD 20)

add_executable(Solitaire main.cpp
        logger.h
        ScreenBuffer.h
        Card.h
        CardStash.h
        ConsoleColors.h
        Renderable.h
        InputBox.h
        Input.h
        SolitaireGame.h
        TableauPile.h
        FoundationPile.h
        Selector.h
        ScoreManager.h
)
