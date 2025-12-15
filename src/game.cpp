#include "game.h"
#include <cstdlib>
#include <iostream>

Game::Game(int level) : difficultyLevel(level) {
    int maxNumber;
    switch (difficultyLevel) {
        case 1: maxNumber = 10; break; // Easy
        case 2: maxNumber = 50; break; // Medium
        case 3: maxNumber = 100; break; // Hard
        default: maxNumber = 10;
    }
    secretNumber = rand() % maxNumber + 1;
}

int Game::getSecretNumber() const {
    return secretNumber;
}

bool Game::checkGuess(int guess) const {
    return guess == secretNumber;
}

float Game::calculatePrize(int attempts) const {
    float basePrize;
    switch (difficultyLevel) {
        case 1: basePrize = 100.0f; break; // Easy
        case 2: basePrize = 500.0f; break; // Medium
        case 3: basePrize = 1000.0f; break; // Hard
        default: basePrize = 100.0f;
    }
    return basePrize / attempts;
}
