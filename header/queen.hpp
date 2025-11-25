#ifndef QUEEN_HPP
#define QUEEN_HPP

#include "../header/piece.hpp"

class Queen : public Piece {
  public:
    Queen(PieceColor color);
    Queen(PieceColor color, bool moved);
    Piece* clone() const override;
};


#endif // QUEEN_HPP