#ifndef KNIGHT_HPP
#define KNIGHT_HPP

#include "../header/piece.hpp"

class Knight : public Piece {
  public:
    Knight(PieceColor color);
    Knight(PieceColor color, bool moved);
    Piece* clone() const override;
};

#endif // KNIGHT_HPP