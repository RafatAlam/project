#include "../header/rook.hpp"
#include "../header/board.hpp"

Rook::Rook(PieceColor color) : Piece(ROOK, color) {}

Rook::Rook(PieceColor color, bool moved) : Piece(ROOK, color, moved) {}

Piece* Rook::clone() const {
  return new Rook(color, moved);
}