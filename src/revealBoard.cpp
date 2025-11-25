#include "../header/revealBoard.hpp"
#include <algorithm>
#include <random>

RevealBoard::RevealBoard() {
  clearBoard();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<PieceType> whitePieces = {
    QUEEN,
    ROOK, ROOK,
    BISHOP, BISHOP,
    KNIGHT, KNIGHT,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN
  };
  std::vector<PieceType> blackPieces = whitePieces;
  std::shuffle(whitePieces.begin(), whitePieces.end(), gen);
  std::shuffle(blackPieces.begin(), blackPieces.end(), gen);

  int count = 0;
  // White Piece
  for (int i=0; i<2; i++) {
    for (int j=0; j<8; j++) {
      if (i == 0 && j == 4)
        addPiece(KING, WHITE, i, j);
      else {
        addPiece(whitePieces[count], WHITE, i, j);
        count++;
      }
    }
  }

  // Black Piece
  count = 0;
  for (int i=7; i>=6; i--) {
    for (int j=0; j<8; j++) {
      if (i == 7 && j == 4)
        addPiece(KING, BLACK, i, j);
      else {
        addPiece(blackPieces[count], BLACK, i, j);
        count++;
      }
    }
  }
}

PieceType RevealBoard::getPieceType(int row, int col) const {
  if (pieceMoved(row, col))
    return chessBoard[row][col]->getType();
  else {
    return getInitialPieceType(row, col);
  }
}

Board* RevealBoard::clone() const {
  return new RevealBoard(*this);
}