#include "../header/piece.hpp"

Piece::Piece(PieceType type, PieceColor color) : type(type), color(color) {}

Piece::Piece(PieceType type, PieceColor color, bool moved) : type(type), color(color), moved(moved) {}

PieceType Piece::getType() const {
  return this->type;
}


PieceColor Piece::getColor() const {
  return this->color;
}


bool Piece::getMoved() const {
  return this->moved;
}


void Piece::setMoved() {
  this->moved = true;
}


bool Piece::getAlive() const{
  return this->alive;
}


void Piece::setAlive(){
  this->alive = false;
}

/*
Position Piece::getPosition() const {
  return this->position;
}


void Piece::changePosition(int x, int y) {
  position.move(x,y);
}
*/