#include "../header/game.hpp"

Game::Game(Board* board) : board(board), currTurn(WHITE), state(INPROGRESS) {}

PieceColor Game::getCurrentTurn() const {
  return currTurn;
}

Board* Game::getBoard(){
  return board;
}

GameState Game::getGameState() const{
  return state;
}

bool Game::isMoveLegal(int srcRow, int srcCol, int dstRow, int dstCol) {
  if (!isCurrentPlayerPiece(srcRow, srcCol))
    return false;
  
  std::vector<Position> moves = board->validMoves(srcRow, srcCol);
  if (std::find(moves.begin(), moves.end(), Position(dstRow, dstCol)) != moves.end())
    return true;
  return false;
}

bool Game::makeMove(int srcRow, int srcCol, int dstRow, int dstCol) {
  if (!isMoveLegal(srcRow, srcCol, dstRow, dstCol))
    return false;
  
  PieceType type = board->getPieceType(srcRow, srcCol);

  // Castling
  if (type == KING && srcRow == dstRow && (dstCol == 6 || dstCol == 2)) {
    if (dstCol == 6) { // king-side rook: (row=sr, col 7 -> 5)
      if (board->isOccupied(srcRow, 7) && board->getPieceType(srcRow, 7) == ROOK) {
        board->movePiece(srcRow, 7, srcRow, 5);
        board->pieceSetMoved(srcRow, 5);
      }
    }
    else { // queen-side rook: (row=sr, col 0 -> 3)
      if (board->isOccupied(srcRow, 0) && board->getPieceType(srcRow, 0) == ROOK) {
        board->movePiece(srcRow, 0, srcRow, 3);
        board->pieceSetMoved(srcRow, 3);
      }
    }
  }

  // Move
  board->movePiece(srcRow, srcCol, dstRow, dstCol);
  switchTurn();
  evaluateGameState();
  return true;
}

bool Game::isCurrentPlayerPiece(int row, int col) const {
  if (!board->isOccupied(row, col))
    return false;
  return (board->getColor(row, col) == currTurn);
}

void Game::switchTurn() {
  if (currTurn == WHITE)
    currTurn = BLACK;
  else
    currTurn = WHITE;
}

void Game::evaluateGameState() {
  bool inCheck = board->kingInCheck(currTurn);
  bool hasMove = false;

  // Check all squares on the board
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; ++j) {
      if (!board->isOccupied(i, j)) 
        continue;
      if (board->getColor(i, j) != currTurn) 
        continue;

      std::vector<Position> moves = board->validMoves(i, j);
      if (!moves.empty()) {
        hasMove = true;
        break;
      }
    }
    if (hasMove)
      break;
  }

  if (hasMove) {
    if (inCheck)
      state = CHECK;
    else
      state = INPROGRESS;
  } 
  else {
    if (inCheck) {
      state = CHECKMATE;
    } else {
      state = DRAW;
    }
  }
}