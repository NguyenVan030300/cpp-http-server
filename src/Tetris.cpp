#include "Tetris.hpp"

int Tetris::rotation(int px, int py, int r) {
  int pi = 0;
  switch (r % 4) {
    case 0:              // 0 degrees			
						 // 0  1  2  3
      pi = py * 4 + px;  // 4  5  6  7
      break;             // 8  9 10 11
                         // 12 13 14 15

    case 1:                     // 90 degrees			
								//12  8  4  0
      pi = 12 + py - (px * 4);  // 13  9  5  1
      break;                    // 14 10  6  2
                                // 15 11  7  3

    case 2:                     // 180 degrees			
								//15 14 13 12
      pi = 15 - (py * 4) - px;  // 11 10  9  8
      break;                    // 7  6  5  4
                                // 3  2  1  0

    case 3:                    // 270 degrees			
							   // 3  7 11 15
      pi = 3 - py + (px * 4);  // 2  6 10 14
      break;                   // 1  5  9 13
  }                            // 0  4  8 12

  return pi;
}

char Tetris::getBlock(int index, int stringIndex) {
  return this->pieces[index][stringIndex];
}

bool Tetris::doesPieceFit(int nPiece, int nRotation, int nPosX, int nPosY) {
  // 1. Loop through the 4x4 box of the piece
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      // Step A: Get the character index inside the rotated piece string
      int pi = rotation(px, py, nRotation);

      // If this cell in the 4x4 box is empty ('.'), we don't care about it!
      if (pieces[nPiece][pi] == '.') {
        continue;
      }

      // Step B: Calculate where this specific block lands on the real board
      int boardX = nPosX + px;
      int boardY = nPosY + py;

      // Step C: Check Rule 1 & 2 - Is it out of bounds horizontally or too low?
      if (boardX < 1 || boardX >= fieldWidth - 1 || boardY >= fieldHeight - 1) {
        return false;  // Hit a wall or the floor!
      }

      // Step D: Check if it's above the ceiling (boardY < 0 is allowed during
      // spawn)
      if (boardY >= 0) {
        // Step E: Check Rule 3 - Did it hit an already locked block on the
        // field? (Assuming 0 means empty space, and anything > 0 means a locked
        // block)
        if (field[boardY * fieldWidth + boardX] != 0) {
          return false;  // Hit another block!
        }
      }
    }
  }

  // If the loop finishes without returning false, the piece fits safely!
  return true;
}

void Tetris::lockPiece(int nPiece, int nRotation, int nPosX, int nPosY) {
  // 1. Loop through the 4x4 box of the falling piece
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      // Get index into the piece string
      int pi = rotation(px, py, nRotation);

      // If there is an actual block here (not a dot)
      if (pieces[nPiece][pi] != '.') {
        // Calculate where it lands on the real board
        int boardX = nPosX + px;
        int boardY = nPosY + py;

        // Make sure it's safely inside the board boundaries before writing
        if (boardY >= 0 && boardY < fieldHeight - 1 && boardX >= 0 &&
            boardX < fieldWidth - 1) {
          field[boardY * fieldWidth + boardX] = pieces[nPiece][pi];
        }
      }
    }
  }
}

int Tetris::clearLines() {
  int clearedLines = 0;
  int y = fieldHeight - 2;

  while (y >= 0) {
    bool lineIsFull = true;
    for (int x = 1; x < fieldWidth - 1; ++x) {
      if (field[y * fieldWidth + x] == 0) {
        lineIsFull = false;
        break;
      }
    }

    if (!lineIsFull) {
      --y;
      continue;
    }

    for (int row = y; row > 0; --row) {
      for (int x = 1; x < fieldWidth - 1; ++x) {
        field[row * fieldWidth + x] = field[(row - 1) * fieldWidth + x];
      }
    }
    for (int x = 1; x < fieldWidth - 1; ++x) {
      field[x] = 0;
    }
    ++clearedLines;
  }

  return clearedLines;
}
