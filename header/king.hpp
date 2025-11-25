#ifndef KING_HPP
#define KING_HPP

#include "../header/piece.hpp"

class King : public Piece {
  public:
    King(PieceColor color);
    King(PieceColor color, bool moved);
    Piece* clone() const override;
};

#endif // KING_HPP