#include <gtest/gtest.h>

#include "Tetris.hpp"

TEST(TetrisTest, clearsCompleteLineAndReturnsLineCount) {
  Tetris tetris;
  tetris.lockPiece(0, 1, 1, 16);
  tetris.lockPiece(0, 1, 5, 16);
  tetris.lockPiece(0, 0, 7, 15);

  EXPECT_EQ(tetris.clearLines(), 1);
  for (int x = 1; x < tetris.getFieldWidth() - 2; ++x) {
    EXPECT_EQ(tetris.getField(x, 18), 0);
  }
  EXPECT_EQ(tetris.getField(9, 18), 'L');
}
