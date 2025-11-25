#ifndef PAWN_HPP
#define PAWN_HPP

#include "../header/piece.hpp"

class Pawn : public Piece {
  public:
    Pawn(PieceColor color);
    Pawn(PieceColor color, bool moved);
    Piece* clone() const override;
};


#endif // PAWN_HPP