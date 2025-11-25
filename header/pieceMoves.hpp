#ifndef PIECEMOVE_HPP
#define PIECEMOVE_HPP

#include <vector>
#include "piece.hpp"
#include "board.hpp"

class PieceMoves {
  private:
    Board* board;
  public:
  PieceMoves(Board* board);
  // generate valid moves
  std::vector<Position> validMoves(int row, int col);
  // generate moves according to the coordinate
  std::vector<Position> generateMoves(int row, int col);
  // Removes all moves from moves causing the king to be in check
  void validateMoves(int row, int col, std::vector<Position>& moves);

  std::vector<Position> bishopMoves(int row, int col);
  std::vector<Position> kingMoves(int row, int col);
  std::vector<Position> knightMoves(int row, int col);
  std::vector<Position> pawnMoves(int row, int col);
  std::vector<Position> queenMoves(int row, int col);
  std::vector<Position> rookMoves(int row, int col);

  // If return true, this move involves capturing piece. 
  // Otherwise, just check if the position is available to move.
  bool checkMove(int row, int col, PieceColor color, std::vector<Position>& moves); 
};


#endif // PIECEMOVE_HPP