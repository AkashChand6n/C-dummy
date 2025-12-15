# Casino Number Guessing Game

Welcome to the Casino Number Guessing Game! This is the complete solution for the project.

## How to Play

1. Enter your name.
2. Choose a difficulty level (Easy, Medium, Hard).
3. Guess the secret number within the allowed range.
4. Win money based on your performance!

## Project Structure

- `src/`: Contains the source code for the game.
- `tests/`: Contains unit tests for the game.
- `Jenkinsfile`: Defines the Jenkins pipeline for building, testing, and delivering the game.
- `CMakeLists.txt`: CMake configuration for building the project.

## Getting Started

1. Clone the repository:
   ```
   git clone https://github.com/YOUR-USERNAME/casino-number-guessing-game.git
   cd casino-number-guessing-game
   ```

2. Build the project:
    ```
    cmake -B build -S .
    cmake --build build
    ```

3. Run the game:
    ```
    ./build/casino_game
    ```

4. Run the tests:
    ```
    ./build/test_game
    ```

## Jenkins Pipeline

To run the Jenkins pipeline:

1.Set up Jenkins with the C++ profile.

2.Create a new pipeline job and point it to this repository.

3.Run the pipeline to build, test, and deliver the game.