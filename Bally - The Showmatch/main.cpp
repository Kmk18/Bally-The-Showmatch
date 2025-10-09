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
    std::cout << "Controls:" << std::endl;
    std::cout << "Player 1: Arrow Keys + Space" << std::endl;
    std::cout << "Player 2: WASD + Left Shift" << std::endl;
    std::cout << "Player 3: IJKL + U" << std::endl;
    std::cout << "Player 4: Numpad" << std::endl;
    std::cout << "ESC: Exit, R: Restart (when game over)" << std::endl;

    game.Run();
    game.Shutdown();

    return 0;
}
