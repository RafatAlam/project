#ifndef ROOK_HPP
#define ROOK_HPP

#include "../header/piece.hpp"

class Rook : public Piece {
  public:
    Rook(PieceColor color);
    Rook(PieceColor color, bool moved);
    Piece* clone() const override;
};

#endif // ROOK_HPP