#include "../header/bishop.hpp"
#include "../header/board.hpp"

Bishop::Bishop(PieceColor color) : Piece(BISHOP, color) {}

Bishop::Bishop(PieceColor color, bool moved) : Piece(BISHOP, color, moved) {}

Piece* Bishop::clone() const {
  return new Bishop(color, moved);
}