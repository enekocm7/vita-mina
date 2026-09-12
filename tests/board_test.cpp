#include "board.hpp"

#include <cassert>
#include <iostream>

namespace {
void testFirstRevealAndWin() {
    Board board(9, 9, 10);
    board.reveal(4, 4);

    assert(board.state() == GameState::Playing);
    assert(board.cell(4, 4).revealed);

    int mines = 0;
    for (int row = 0; row < board.rows(); ++row) {
        for (int column = 0; column < board.columns(); ++column) {
            const Cell& cell = board.cell(column, row);
            if (cell.mine) {
                ++mines;
            }
            if (column >= 3 && column <= 5 && row >= 3 && row <= 5) {
                assert(!cell.mine);
            }
        }
    }
    assert(mines == 10);

    for (int row = 0; row < board.rows(); ++row) {
        for (int column = 0; column < board.columns(); ++column) {
            if (!board.cell(column, row).mine) {
                board.reveal(column, row);
            }
        }
    }
    assert(board.state() == GameState::Won);
    assert(board.flagsPlaced() == board.mineCount());
}

void testFlagsAndReset() {
    Board board(16, 16, 40);
    board.toggleFlag(2, 3);
    assert(board.cell(2, 3).flagged);
    assert(board.flagsPlaced() == 1);
    assert(board.remainingMines() == 39);

    board.reveal(2, 3);
    assert(board.state() == GameState::Ready);

    board.toggleFlag(2, 3);
    assert(!board.cell(2, 3).flagged);
    assert(board.flagsPlaced() == 0);

    board.reset(24, 16, 75);
    assert(board.columns() == 24);
    assert(board.rows() == 16);
    assert(board.mineCount() == 75);
    assert(board.state() == GameState::Ready);
}
} // namespace

int main() {
    testFirstRevealAndWin();
    testFlagsAndReset();
    std::cout << "Board tests passed\n";
    return 0;
}
