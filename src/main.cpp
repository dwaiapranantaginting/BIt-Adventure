#include "Game.h"
#include <ctime>
#include <cstdlib>

int main() {
    bool playAgain = true;

    while (playAgain) {
        Game game;      // Buat dunia game dari nol (HP, musuh, map otomatis kereset 100%)
        game.run();     // Mainkan gamenya
        
        // Setelah game.run() selesai (karena mati/quit), cek apakah user nekan 'R'
        playAgain = game.requestRestart; 
    }

    return 0;
}