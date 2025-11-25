#include "../header/knight.hpp"
#include "../header/board.hpp"

Knight::Knight(PieceColor color) : Piece(KNIGHT, color) {}

Knight::Knight(PieceColor color, bool moved) : Piece(KNIGHT, color, moved) {}

Piece* Knight::clone() const {
  return new Knight(color, moved);
}