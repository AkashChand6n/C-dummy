#ifndef GAME_H
#define GAME_H

class Game {
private:
    int secretNumber;
    int difficultyLevel;
public:
    Game(int level);
    int getSecretNumber() const;
    bool checkGuess(int guess) const;
    float calculatePrize(int attempts) const;
};

#endif
