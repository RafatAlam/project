#ifndef REVEALBOARD_HPP
#define REVEALBOARD_HPP

#include "../header/board.hpp"

class RevealBoard : public Board {
  public:
    RevealBoard();
    PieceType getPieceType(int row, int col) const override;
    Board* clone() const override;
};


#endif // REVEALBOARD_HPP