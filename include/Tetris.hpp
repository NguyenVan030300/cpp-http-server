#pragma once
#include <string>
#include <vector>

class Tetris {
 public:
  Tetris() {
    // Long piece
    pieces[0] += "..L.";
    pieces[0] += "..L.";
    pieces[0] += "..L.";
    pieces[0] += "..L.";

    // T piece
    pieces[1] += "....";
    pieces[1] += ".TTT";
    pieces[1] += "..T.";
    pieces[1] += "....";

    // S piece
    pieces[2] += ".S..";
    pieces[2] += ".SS.";
    pieces[2] += "..S.";
    pieces[2] += "....";

    // S piece rev
    pieces[3] += "..G.";
    pieces[3] += ".GG.";
    pieces[3] += ".G..";
    pieces[3] += "....";

    // L piece
    pieces[4] += "....";
    pieces[4] += ".AAA";
    pieces[4] += ".A..";
    pieces[4] += "....";

    // L piece rev
    pieces[5] += "....";
    pieces[5] += ".BBB";
    pieces[5] += "...B";
    pieces[5] += "....";

    // square piece rev
    pieces[5] += "....";
    pieces[5] += ".XX.";
    pieces[5] += ".XX.";
    pieces[5] += "....";

    // z piece rev
    pieces[6] += "....";
    pieces[6] += ".ZZ.";
    pieces[6] += "..ZZ";
    pieces[6] += "....";
    field.resize(fieldWidth * fieldHeight, 0);
  };
  int getFieldWidth() { return this->fieldWidth; };
  int getFieldHeight() { return this->fieldHeight; };
  char getField(int x, int y) const {
    if (x < 0 || x >= fieldWidth - 1 || y < 0 || y >= fieldHeight - 1) return 0;
    return field[y * fieldWidth + x];
  }
  int rotation(int px, int py, int r);
  char getBlock(int index, int stringIndex);
  bool doesPieceFit(int piece, int nRotation, int nPosX, int nPosY);
  void lockPiece(int nPiece, int nRotation, int nPosX, int nPosY);
  int clearLines();

 private:
  int fieldWidth = 11;
  int fieldHeight = 21;
  std::string pieces[7];
  std::vector<char> field;  // Stores locked block characters on the board!
};