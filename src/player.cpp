#include "player.h"

Player::Player(std::string playerName) : name(playerName), totalWinnings(0.0f) {}

void Player::addWinnings(float amount) {
    totalWinnings += amount;
}

float Player::getTotalWinnings() {
    return totalWinnings;
}
