#include "../header/king.hpp"
#include "../header/board.hpp"

King::King(PieceColor color) : Piece(KING, color) {}

King::King(PieceColor color, bool moved) : Piece(KING, color, moved) {}

Piece* King::clone() const {
  return new King(color, moved);
}