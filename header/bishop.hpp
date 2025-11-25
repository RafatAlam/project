#ifndef BISHOP_HPP
#define BISHOP_HPP

#include "../header/piece.hpp"

class Bishop : public Piece {
  public:
    Bishop(PieceColor color);
    Bishop(PieceColor color, bool moved);
    Piece* clone() const override;
};

#endif // BISHOP_HPP