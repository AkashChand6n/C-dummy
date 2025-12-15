#include <iostream>
#include "../src/game.h"
#include <cassert>

void testGame() {
    Game game(1); // Easy difficulty

    int secretNumber = game.getSecretNumber();

    assert(secretNumber >= 1 && secretNumber <= 10);

    int wrongGuess = (secretNumber == 10) ? 9 : secretNumber + 1;  
    assert(game.checkGuess(wrongGuess) == false);

    assert(game.checkGuess(secretNumber) == true);
}

int main() {
    testGame();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}