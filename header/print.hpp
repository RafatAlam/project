#ifndef HELPER_HPP
#define HELPER_HPP

#include "board.hpp"
#include <string>
#include <iostream>

using namespace std;


class Print {
  public:
    void printBoard(const Board& board);
    string pieceSymbol(const Board& board, int row, int col);
    void printMoves(Board& board, int row, int col);
    void printKingStatus(Board& board);
};

#endif // HELPER_HPP