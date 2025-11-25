#include "../header/pieceMoves.hpp"

PieceMoves::PieceMoves(Board* board) : board(board) {}

std::vector<Position> PieceMoves::validMoves(int row, int col) {
  std::vector<Position> moves;
  if (!(board->isOccupied(row, col)))
    return {};
  moves = generateMoves(row, col);
  validateMoves(row, col, moves);
  return moves;
}

std::vector<Position> PieceMoves::generateMoves(int row, int col) {
  std::vector<Position> moves;
  PieceType type = board->getPieceType(row, col);
  switch (type) {
  case BISHOP:
    moves = bishopMoves(row, col);
    break;
  case KING:
    moves = kingMoves(row, col);
    break;
  case KNIGHT:
    moves = knightMoves(row, col);
    break;
  case PAWN:
    moves = pawnMoves(row, col);
    break;
  case QUEEN:
    moves = queenMoves(row, col);
    break;
  case ROOK:
    moves = rookMoves(row, col);
    break;
  default:
    moves = {};
  }
  return moves;
}


std::vector<Position> PieceMoves::bishopMoves(int row, int col) {
  std::vector<Position> possibleMoves;
  PieceColor color = board->getColor(row, col);

  // up right
  for(int i = row+1, j=col+1; i<8 && j<8; i++, j++) {
    if (checkMove(i, j, color, possibleMoves))
      break;
  }

  // down right
  for(int i = row-1, j=col+1; i>=0 && j<8; i--, j++) {
    if (checkMove(i, j, color, possibleMoves))
      break;
  }

  // up left
  for(int i = row+1, j=col-1; i<8 && j>=0; i++, j--) {
    if (checkMove(i, j, color, possibleMoves))
      break;
  }

  // down left
  for(int i = row-1, j=col-1; i>=0 && j>=0; i--, j--) {
    if (checkMove(i, j, color, possibleMoves))
      break;
  }

  return possibleMoves;
}

std::vector<Position> PieceMoves::kingMoves(int row, int col) {
  std::vector<Position> possibleMoves;
  PieceColor color = board->getColor(row, col);

  // up
  if (row+1<8)
    checkMove(row+1, col, color, possibleMoves);

  // down
  if (row-1>=0)
    checkMove(row-1, col, color, possibleMoves);

  // right
  if (col+1<8)
    checkMove(row, col+1, color, possibleMoves);

  // left
  if (col-1>=0)
    checkMove(row, col-1, color, possibleMoves);

  // up right
  if (row+1<8 && col+1<8)
    checkMove(row+1, col+1, color, possibleMoves);

  // up left
  if (row+1<8 && col-1>=0)
    checkMove(row+1, col-1, color, possibleMoves);

  // down right
  if (row-1>=0 && col+1<8)
    checkMove(row-1, col+1, color, possibleMoves);

  // down left
  if (row-1>=0 && col-1>=0)
    checkMove(row-1, col-1, color, possibleMoves);

  // castling
  if (!(board->pieceMoved(row, col))) {
    // right
    for (int i=col+1; i<8; i++) {
      if (board->isOccupied(row, i)) {
        if (i == 7) {
          if (board->getColor(row, i) == color && board->getPieceType(row, i) == ROOK && !board->pieceMoved(row, i)) {
            possibleMoves.push_back(Position(row, 6));
          }
        }
        else
          break;
      }
    }
    // left
    for (int i=col-1; i>=0; i--) {
      if (board->isOccupied(row, i)) {
        if (i == 0) {
          if (board->getColor(row, i) == color && board->getPieceType(row, i) == ROOK && !board->pieceMoved(row, i)) {
            possibleMoves.push_back(Position(row, 2));
          }
        }
        else
          break;
      }
    }
  }

  return possibleMoves;
}
std::vector<Position> PieceMoves::knightMoves(int row, int col) {
  std::vector<Position> possibleMoves;
  PieceColor color = board->getColor(row, col);

  // up 2, right 1
  if (row+2<8 && col+1<8)
    checkMove(row+2, col+1, color, possibleMoves);

  // up 1, right 2
  if (row+1<8 && col+2<8)
    checkMove(row+1, col+2, color, possibleMoves);

  // down 2, right 1
  if (row-2>=0 && col+1<8) 
    checkMove(row-2, col+1, color, possibleMoves);

  // down 1, right 2
  if (row-1>=0 && col+2<8)
    checkMove(row-1, col+2, color, possibleMoves);

  // up 2, left 1
  if (row+2<8 && col-1>=0)
    checkMove(row+2, col-1, color, possibleMoves);

  // up 1, left 2
  if (row+1<8 && col-2>=0)
    checkMove(row+1, col-2, color, possibleMoves);

  // down 2, left 1
  if (row-2>=0 && col-1>=0)
    checkMove(row-2, col-1, color, possibleMoves);

  // down 1, left 2
  if (row-1>=0 && col-2>=0)
    checkMove(row-1, col-2, color, possibleMoves);

  return possibleMoves;
}

std::vector<Position> PieceMoves::pawnMoves(int row, int col) {
  std::vector<Position> possibleMoves;
  PieceColor color = board->getColor(row, col);

  int direction = 1;
  if (color == BLACK)
    direction = -1;
  
  if ((direction == 1 && row < 7) || (direction == -1 && row > 0)) {
    if ((board->isOccupied(row+direction,col+1)) && board->getColor(row+direction,col+1) != color)
      possibleMoves.push_back(Position(row+direction,col+1));
    if ((board->isOccupied(row+direction,col-1)) && board->getColor(row+direction,col-1) != color)
      possibleMoves.push_back(Position(row+direction,col-1));

    if (!(board->isOccupied(row+direction,col)))
      possibleMoves.push_back(Position(row+direction,col));
    else
      return possibleMoves;
  }
  
  if (!(board->pieceMoved(row, col))) {
    if (!(board->isOccupied(row+2*direction,col)))
      possibleMoves.push_back(Position(row+2*direction,col));
  }

  return possibleMoves;
}

std::vector<Position> PieceMoves::queenMoves(int row, int col) {
  std::vector<Position> possibleMoves = rookMoves(row, col);
  std::vector<Position> diagonal = bishopMoves(row, col);
  possibleMoves.insert(possibleMoves.end(), diagonal.begin(), diagonal.end());

  return possibleMoves;
}

std::vector<Position> PieceMoves::rookMoves(int row, int col) {
  std::vector<Position> possibleMoves;
  PieceColor color = board->getColor(row, col);
  // right 
  for(int j = col+1; j < 8; j++) {
    if (checkMove(row, j, color, possibleMoves))
      break;
  }

  // left
  for(int j = col-1; j >= 0 ; j--) {
    if (checkMove(row, j, color, possibleMoves))
      break;
  }

  // down
  for(int i = row-1; i >= 0 ; i--) {
    if (checkMove(i, col, color, possibleMoves))
      break;
  }

  // up
  for(int i = row+1; i < 8 ; i++) {
    if (checkMove(i, col, color, possibleMoves))
      break;
  }
  
  // Haven't added castling

  return possibleMoves;
}

bool PieceMoves::checkMove(int row, int col, PieceColor color,std::vector<Position>& moves) {
  if (!(board->isOccupied(row,col))) {
    moves.push_back(Position(row,col));
  }  
  else {
    if ((board->getColor(row,col) != color)) {
      moves.push_back(Position(row,col));
    } 
    return true;
  }
  return false;
}

void PieceMoves::validateMoves(int row, int col, std::vector<Position>& moves) {
  if (moves.size() == 0)
    return;
  
  PieceColor color = board->getColor(row, col);
  
  for (int i=0; i<moves.size();) {
    auto newBoard = board->clone();
    Position move = moves[i];
    newBoard->movePiece(row, col, move.x, move.y);
    if (newBoard->kingInCheck(color)) {
      moves.erase(moves.begin() + i);
    }
    else
      i++;
  }
}