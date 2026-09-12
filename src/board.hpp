#pragma once

#include <cstdint>
#include <random>
#include <vector>

enum class GameState {
    Ready,
    Playing,
    Won,
    Lost,
};

struct Cell {
    bool mine = false;
    bool revealed = false;
    bool flagged = false;
    std::uint8_t adjacent = 0;
};

class Board {
public:
    Board(int columns, int rows, int mineCount);

    void reset(int columns, int rows, int mineCount);
    void reveal(int column, int row);
    void toggleFlag(int column, int row);

    [[nodiscard]] int columns() const { return columns_; }
    [[nodiscard]] int rows() const { return rows_; }
    [[nodiscard]] int mineCount() const { return mineCount_; }
    [[nodiscard]] int flagsPlaced() const { return flagsPlaced_; }
    [[nodiscard]] int remainingMines() const { return mineCount_ - flagsPlaced_; }
    [[nodiscard]] GameState state() const { return state_; }
    [[nodiscard]] const Cell& cell(int column, int row) const;

private:
    [[nodiscard]] bool inBounds(int column, int row) const;
    [[nodiscard]] int index(int column, int row) const;
    void placeMines(int safeColumn, int safeRow);
    void revealEmptyArea(int column, int row);
    void revealAllMines();
    void checkForWin();

    int columns_ = 0;
    int rows_ = 0;
    int mineCount_ = 0;
    int flagsPlaced_ = 0;
    int revealedSafeCells_ = 0;
    GameState state_ = GameState::Ready;
    std::vector<Cell> cells_;
    std::mt19937 random_;
};
