#ifndef PIECE_HPP
#define PIECE_HPP

#include <vector>

class Board;

enum PieceType {
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
};

enum PieceColor {
  BLACK,
  WHITE,
};

struct Position {
  int x;
  int y;
  Position() : Position(0,0) {}
  Position(int x, int y) : x(x),  y(y) {}

  bool operator==(const Position& rhs) const {
    return (x == rhs.x && y == rhs.y);
  }

  bool operator!=(const Position& rhs) const {
    return !(*this == rhs);
  }
};

class Piece {
  protected:
    PieceType type;
    PieceColor color;
    //Position position;
    bool moved = false;
    bool alive = true;
  public:
    Piece(PieceType type, PieceColor color);
    Piece(PieceType type, PieceColor color, bool moved);
    virtual ~Piece() = default;
    PieceType getType() const;
    PieceColor getColor() const;
    bool getMoved() const;
    void setMoved();
    bool getAlive() const;
    void setAlive();
    //Position getPosition() const;
    //void changePosition(int x, int y);
    virtual Piece* clone() const = 0;
};


#endif // PIECE_HPP