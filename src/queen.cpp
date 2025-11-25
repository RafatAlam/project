#include "../header/queen.hpp"
#include "../header/board.hpp"

Queen::Queen(PieceColor color) : Piece(QUEEN, color) {}

Queen::Queen(PieceColor color, bool moved) : Piece(QUEEN, color, moved) {}

Piece* Queen::clone() const {
  return new Queen(color, moved);
}