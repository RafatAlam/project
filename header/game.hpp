#ifndef GAME_HPP
#define GAME_HPP

#include <vector>

#include "piece.hpp"
#include "revealBoard.hpp"

enum GameState {
  INPROGRESS,
  CHECK,
  CHECKMATE,
  DRAW,
};

class Game {
  private:
    Board* board = nullptr;
    PieceColor currTurn;
    GameState state;
  public:
    Game(Board* board);
    PieceColor getCurrentTurn() const;
    Board* getBoard();
    GameState getGameState() const;
    bool isMoveLegal(int srcRow, int srcCol, int dstRow, int dstCol);
    bool makeMove(int srcRow, int srcCol, int dstRow, int dstCol);
    bool isCurrentPlayerPiece(int row, int col) const;
    void switchTurn();
    void evaluateGameState();
};

#endif // GAME_HPP