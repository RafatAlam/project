#include "../header/pawn.hpp"
#include "../header/board.hpp"

Pawn::Pawn(PieceColor color) : Piece(PAWN, color) {}

Pawn::Pawn(PieceColor color, bool moved) : Piece(PAWN, color, moved) {}

Piece* Pawn::clone() const {
  return new Pawn(color, moved);
}