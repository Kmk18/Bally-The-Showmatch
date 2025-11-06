#include "Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Game game;

    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    std::cout << "Bally - The Showdown" << std::endl;
    std::cout << "Starting game..." << std::endl;

    game.Run();
    game.Shutdown();

    std::cout << "Game exited!" << std::endl;

    return 0;
}
