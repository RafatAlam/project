#include "../header/board.hpp"
#include "../header/pieceMoves.hpp"

Board::Board() {
  clearBoard();
  addPiece(ROOK, WHITE, 0, 0);
  addPiece(KNIGHT, WHITE, 0, 1);
  addPiece(BISHOP, WHITE, 0, 2);
  addPiece(QUEEN, WHITE, 0, 3);
  addPiece(KING, WHITE, 0, 4);
  addPiece(BISHOP, WHITE, 0, 5);
  addPiece(KNIGHT, WHITE, 0, 6);
  addPiece(ROOK, WHITE, 0, 7);
  for (int i=0; i<8; i++) {
    addPiece(PAWN, WHITE, 1, i);
  }

  addPiece(ROOK, BLACK, 7, 0);
  addPiece(KNIGHT, BLACK, 7, 1);
  addPiece(BISHOP, BLACK, 7, 2);
  addPiece(QUEEN, BLACK, 7, 3);
  addPiece(KING, BLACK, 7, 4);
  addPiece(BISHOP, BLACK, 7, 5);
  addPiece(KNIGHT, BLACK, 7, 6);
  addPiece(ROOK, BLACK, 7, 7);
  for (int i=0; i<8; i++) {
    addPiece(PAWN, BLACK, 6, i);
  }
}

Board::Board(const Board& rhs) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (!(rhs.isOccupied(i,j)))
        continue;
      chessBoard[i][j] = rhs.getPiece(i,j)->clone();
    }
  }
}

Board::~Board() {
  for(int i=0; i<8; i++) {
    for(int j=0; j<8; j++) {
      if (chessBoard[i][j] != nullptr)
        delete chessBoard[i][j];
    }
  }
}
    
bool Board::isOccupied(int row, int col) const {
  return (chessBoard[row][col] != nullptr);
}
    
PieceColor Board::getColor(int row, int col) const {
  return chessBoard[row][col]->getColor();
}
   
PieceType Board::getPieceType(int row, int col) const {
  return chessBoard[row][col]->getType();
}

Board* Board::clone() const {
  return new Board(*this);
}

std::vector<Position> Board::validMoves(int row, int col) {
  PieceMoves moves(this);
  return moves.validMoves(row, col);
}

std::vector<Position> Board::generateMoves(int row, int col) {
  PieceMoves moves(this);
  return moves.generateMoves(row, col);
}

void Board::clearBoard() {
  for(int i=0; i<8; i++) {
    for(int j=0; j<8; j++) {
      if (chessBoard[i][j] != nullptr) {
        delete chessBoard[i][j];
        chessBoard[i][j] = nullptr;
      }
    }
  }
}

void Board::addPiece(PieceType type, PieceColor color, int row, int col) {
  if (chessBoard[row][col] != nullptr)
    delete chessBoard[row][col];
  
  chessBoard[row][col] = makeNewPiece(type, color);
}

Piece* Board::getPiece(int row, int col) const {
  return chessBoard[row][col];
}

Piece* Board::makeNewPiece(PieceType type, PieceColor color) {
  switch (type) {
    case BISHOP: 
      return new Bishop(color);
    case KING:   
      return new King(color);
    case KNIGHT: 
      return new Knight(color);
    case PAWN:   
      return new Pawn(color);
    case QUEEN:  
      return new Queen(color);
    case ROOK:   
      return new Rook(color);
  }
  return nullptr;
}

bool Board::pieceMoved(int row, int col) const {
  if (chessBoard[row][col])
    return chessBoard[row][col]->getMoved();
  return true;
}

void Board::pieceSetMoved(int row, int col) {
  if (chessBoard[row][col])
    chessBoard[row][col]->setMoved();
}

void Board::movePiece(int srcRow, int srcCol, int dstRow, int dstCol) {
  if (chessBoard[dstRow][dstCol])
    delete chessBoard[dstRow][dstCol];
  if (chessBoard[srcRow][srcCol]) {
    chessBoard[dstRow][dstCol] = chessBoard[srcRow][srcCol];
    chessBoard[srcRow][srcCol] = nullptr;
    pieceSetMoved(dstRow, dstCol);
  }
}

bool Board::kingInCheck(PieceColor color) {
  Position king_pos = findKing(color);
  if (king_pos.x == 8 && king_pos.y == 8)
    return false; // no king on board

  PieceType type;
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      if (chessBoard[i][j]) {
        if (getColor(i, j) == color)
          continue;
        else {
          std::vector<Position> moves = generateMoves(i, j);
          if (std::find(moves.begin(), moves.end(), king_pos) != moves.end()) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

Position Board::findKing(PieceColor color) {
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      if (chessBoard[i][j]) {
        if (getPieceType(i, j) == KING && getColor(i,j) == color)
          return Position(i, j);
      }
    }
  }
  return Position(8,8);
}

PieceType Board::getInitialPieceType(int row, int col) const {
  if (row == 1 or row == 6)
    return PAWN;
  else {
    switch (col)
    {
    case 0:
    case 7:
      return ROOK;
    case 1:
    case 6:
      return KNIGHT;
    case 2:
    case 5:
     return BISHOP;
    case 3:
      return QUEEN;
    default:
      return KING;
    }
  }
}