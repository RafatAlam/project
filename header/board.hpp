#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <algorithm>
#include "piece.hpp"
#include "bishop.hpp"
#include "king.hpp"
#include "knight.hpp"
#include "pawn.hpp"
#include "queen.hpp"
#include "rook.hpp"
#include "power.hpp"


class Board {
  protected:
    // first row, second column
    Piece* chessBoard[8][8] = {nullptr}; 
  public:
    Board();
    Board(const Board& rhs);
    ~Board();

    Board(Board&&) = default;
    Board& operator=(Board&&) = default;

    bool isOccupied(int row, int col) const;
    PieceColor getColor(int row, int col) const;
    virtual PieceType getPieceType(int row, int col) const;
    virtual Board* clone() const;
    std::vector<Position> validMoves(int row, int col);
    std::vector<Position> generateMoves(int row, int col);
    void clearBoard();
    void addPiece(PieceType type, PieceColor color, int row, int col);
    Piece* getPiece(int row, int col) const;
    Piece* makeNewPiece(PieceType type, PieceColor color);
    bool pieceMoved(int row, int col) const;
    void pieceSetMoved(int row, int col);
    void movePiece(int srcRow, int srcCol, int dstRow, int dstCol);
    bool kingInCheck(PieceColor color);
    Position findKing(PieceColor color);
    // returns the PieceType for initial Board
    PieceType getInitialPieceType(int row, int col) const;
};

#endif // BOARD_HPP