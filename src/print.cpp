#include "../header/print.hpp"

void Print::printBoard(const Board& board) {
  string showBoard;
  for (int i=7; i>=0; i--) {
    for (int j=0; j<8; j++) {
      if (board.isOccupied(i,j)) {
        showBoard += pieceSymbol(board, i, j);
      }
      else {
        showBoard += "[ ]";
      }
      showBoard += " ";
    }
    showBoard += "\n";
  }
  showBoard += "\n\n";
  cout << showBoard;
}

string Print::pieceSymbol(const Board& board, int row, int col) {
  string symbol;
  if (board.getColor(row,col) == WHITE) {
    switch (board.getPieceType(row,col))
    {
    case BISHOP:
      symbol = "[♝]";
      break;
    case KING:
      symbol = "[♚]";
      break;
    case KNIGHT:
      symbol = "[♞]";
      break;
    case PAWN:
      symbol = "[♟]";
      break;
    case QUEEN:
      symbol = "[♛]";
      break;
    case ROOK:
      symbol = "[♜]";
      break;
    default:
      break;
    }
  }
  else {
    switch (board.getPieceType(row,col))
    {
    case BISHOP:
      symbol = "[♗]";
      break;
    case KING:
      symbol = "[♔]";
      break;
    case KNIGHT:
      symbol = "[♘]";
      break;
    case PAWN:
      symbol = "[♙]";
      break;
    case QUEEN:
      symbol = "[♕]";
      break;
    case ROOK:
      symbol = "[♖]";
      break;
    default:
      break;
    }
  }
  return symbol;
}

void Print::printMoves(Board& board, int row, int col) { 
  std::vector<Position> moves = board.validMoves(row, col);
  if (moves.empty())
    cout << "No moves\n";
  else {
    bool boardMoves[8][8] = {}; 
        
    for (int i=0; i<moves.size(); i++) {
      boardMoves[moves[i].x][moves[i].y] = true;
    }
    string showBoard;
    for (int i=7; i>=0; i--) {
      for (int j=0; j<8; j++) {
        if (boardMoves[i][j]) {
          showBoard += "[X]";
        }
        else if (i==row && j==col) {
          showBoard += pieceSymbol(board, row, col);
        }
        else {
          showBoard += "[ ]";
        }
        showBoard += " ";
      }
      showBoard += "\n";
    }
    showBoard += "\n\n";
    cout << showBoard;
  }
}

void Print::printKingStatus(Board& board) {
  if (board.kingInCheck(WHITE)) {
    std::cout << "White King is in check.\n";
  }
  else if (board.kingInCheck(BLACK)) {
    std::cout << "Black King is in check.\n";
  }
  else
    std::cout << "Both Kings are safe.\n";
}