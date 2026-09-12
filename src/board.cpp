#include "board.hpp"

#include <algorithm>
#include <chrono>
#include <queue>
#include <stdexcept>

Board::Board(int columns, int rows, int mineCount)
    : random_(static_cast<std::mt19937::result_type>(
          std::chrono::steady_clock::now().time_since_epoch().count())) {
    reset(columns, rows, mineCount);
}

void Board::reset(int columns, int rows, int mineCount) {
    if (columns <= 0 || rows <= 0 || mineCount <= 0 || mineCount >= columns * rows) {
        throw std::invalid_argument("Invalid Minesweeper board dimensions");
    }

    columns_ = columns;
    rows_ = rows;
    mineCount_ = mineCount;
    flagsPlaced_ = 0;
    revealedSafeCells_ = 0;
    state_ = GameState::Ready;
    cells_.assign(static_cast<std::size_t>(columns * rows), Cell{});
}

const Cell& Board::cell(int column, int row) const {
    if (!inBounds(column, row)) {
        throw std::out_of_range("Cell coordinates are outside the board");
    }
    return cells_[static_cast<std::size_t>(index(column, row))];
}

bool Board::inBounds(int column, int row) const {
    return column >= 0 && column < columns_ && row >= 0 && row < rows_;
}

int Board::index(int column, int row) const {
    return row * columns_ + column;
}

void Board::placeMines(int safeColumn, int safeRow) {
    std::vector<int> candidates;
    candidates.reserve(cells_.size());

    // Keep the first selected cell and its neighbours clear whenever the board
    // has enough room. This guarantees that the opening move reveals an area.
    for (int row = 0; row < rows_; ++row) {
        for (int column = 0; column < columns_; ++column) {
            if (std::abs(column - safeColumn) <= 1 && std::abs(row - safeRow) <= 1) {
                continue;
            }
            candidates.push_back(index(column, row));
        }
    }

    if (static_cast<int>(candidates.size()) < mineCount_) {
        candidates.clear();
        for (int row = 0; row < rows_; ++row) {
            for (int column = 0; column < columns_; ++column) {
                if (column != safeColumn || row != safeRow) {
                    candidates.push_back(index(column, row));
                }
            }
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), random_);
    for (int i = 0; i < mineCount_; ++i) {
        cells_[static_cast<std::size_t>(candidates[static_cast<std::size_t>(i)])].mine = true;
    }

    for (int row = 0; row < rows_; ++row) {
        for (int column = 0; column < columns_; ++column) {
            Cell& current = cells_[static_cast<std::size_t>(index(column, row))];
            if (current.mine) {
                continue;
            }

            int adjacent = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (inBounds(column + dx, row + dy)
                        && cells_[static_cast<std::size_t>(index(column + dx, row + dy))].mine) {
                        ++adjacent;
                    }
                }
            }
            current.adjacent = static_cast<std::uint8_t>(adjacent);
        }
    }
}

void Board::revealEmptyArea(int column, int row) {
    std::queue<int> pending;
    pending.push(index(column, row));

    while (!pending.empty()) {
        const int currentIndex = pending.front();
        pending.pop();
        Cell& current = cells_[static_cast<std::size_t>(currentIndex)];

        if (current.revealed || current.flagged || current.mine) {
            continue;
        }

        current.revealed = true;
        ++revealedSafeCells_;
        if (current.adjacent != 0) {
            continue;
        }

        const int currentColumn = currentIndex % columns_;
        const int currentRow = currentIndex / columns_;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int nextColumn = currentColumn + dx;
                const int nextRow = currentRow + dy;
                if (inBounds(nextColumn, nextRow)) {
                    const Cell& next = cells_[static_cast<std::size_t>(index(nextColumn, nextRow))];
                    if (!next.revealed && !next.flagged && !next.mine) {
                        pending.push(index(nextColumn, nextRow));
                    }
                }
            }
        }
    }
}

void Board::reveal(int column, int row) {
    if (!inBounds(column, row) || state_ == GameState::Won || state_ == GameState::Lost) {
        return;
    }

    Cell& selected = cells_[static_cast<std::size_t>(index(column, row))];
    if (selected.flagged) {
        return;
    }

    if (state_ == GameState::Ready) {
        placeMines(column, row);
        state_ = GameState::Playing;
    }

    if (selected.revealed) {
        if (selected.adjacent == 0) {
            return;
        }

        int adjacentFlags = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (inBounds(column + dx, row + dy)
                    && cells_[static_cast<std::size_t>(index(column + dx, row + dy))].flagged) {
                    ++adjacentFlags;
                }
            }
        }
        if (adjacentFlags != selected.adjacent) {
            return;
        }

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int nextColumn = column + dx;
                const int nextRow = row + dy;
                if (!inBounds(nextColumn, nextRow)) {
                    continue;
                }
                Cell& next = cells_[static_cast<std::size_t>(index(nextColumn, nextRow))];
                if (next.mine && !next.flagged) {
                    next.revealed = true;
                    state_ = GameState::Lost;
                    revealAllMines();
                    return;
                }
                revealEmptyArea(nextColumn, nextRow);
            }
        }
    } else if (selected.mine) {
        selected.revealed = true;
        state_ = GameState::Lost;
        revealAllMines();
        return;
    } else {
        revealEmptyArea(column, row);
    }

    checkForWin();
}

void Board::toggleFlag(int column, int row) {
    if (!inBounds(column, row) || state_ == GameState::Won || state_ == GameState::Lost) {
        return;
    }

    Cell& selected = cells_[static_cast<std::size_t>(index(column, row))];
    if (selected.revealed) {
        return;
    }

    selected.flagged = !selected.flagged;
    flagsPlaced_ += selected.flagged ? 1 : -1;
}

void Board::revealAllMines() {
    for (Cell& current : cells_) {
        if (current.mine) {
            current.revealed = true;
        }
    }
}

void Board::checkForWin() {
    if (revealedSafeCells_ != columns_ * rows_ - mineCount_) {
        return;
    }

    state_ = GameState::Won;
    flagsPlaced_ = mineCount_;
    for (Cell& current : cells_) {
        if (current.mine) {
            current.flagged = true;
        }
    }
}
