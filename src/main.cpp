#include "Game.h"
#include <ctime>   // Tambahkan ini
#include <cstdlib> // Tambahkan ini

int main() {
    // Beri "bibit" acak berdasarkan jam komputermu sekarang
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Game game;
    game.run();
    return 0;
}