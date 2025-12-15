#include <iostream>
#include <string>
#include <cstdlib>
#include <limits>
#include <iomanip>
#include <ctime>

#include "game.h"
#include "player.h"

int main() {
    std::srand(std::time(0));

    std::cout << "Welcome to the Casino Number Guessing Game!" << std::endl;

    bool isNonInteractive = (std::getenv("JENKINS_HOME") != nullptr);

    std::string playerName;
    int difficulty;

    if (isNonInteractive) {
        playerName = "Jenkins";
        difficulty = 1;
        std::cout << "Running in non-interactive mode. Using default values." << std::endl;
    } else {
        std::cout << "Enter your name: ";
        std::cin >> playerName;

        std::cout << "Choose difficulty (1: Easy, 2: Medium, 3: Hard): ";
        std::cin >> difficulty;

        while (difficulty < 1 || difficulty > 3) {
            std::cout << "Invalid difficulty! Choose 1 (Easy), 2 (Medium), or 3 (Hard): ";
            std::cin >> difficulty;
        }
    }

    Player player(playerName);
    Game game(difficulty);

    int secretNumber = game.getSecretNumber();
    std::cout << "Generated number: " << secretNumber << std::endl;

    int guess, attempts = 0;
    bool hasWon = false;

    while (!hasWon) {
        if (isNonInteractive) {
            guess = rand() % 100 + 1;
        } else {
            std::cout << "Enter your guess: ";
            std::cin >> guess;

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input! Please enter a number." << std::endl;
                continue;
            }
        }

        attempts++;

        if (game.checkGuess(guess)) {
            std::cout << "Congratulations! You guessed the number" << std::endl;
            float prize = game.calculatePrize(attempts);
            player.addWinnings(prize);
            std::cout << "You won $" << std::fixed << std::setprecision(2) << prize << "!" << std::endl;
            hasWon = true;
        } else {
            std::cout << "Wrong guess! Try again." << std::endl;
        }
    }

    return 0;
}