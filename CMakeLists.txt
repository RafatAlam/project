cmake_minimum_required(VERSION 3.16)
project(ChessGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------
# Console program
# ---------------------
add_executable(app
  src/main.cpp
  src/board.cpp
  src/piece.cpp
  src/rook.cpp
  src/knight.cpp
  src/bishop.cpp
  src/queen.cpp
  src/king.cpp
  src/pawn.cpp
  src/print.cpp
  src/pieceMoves.cpp
  src/revealBoard.cpp
  src/game.cpp
)
target_include_directories(app PRIVATE header)

# ---------------------
# Web GUI 
# Build: cmake -S . -B build && cmake --build build
# Run:   ./build/web_gui  then open http://localhost:8080
# If remote over SSH: ssh -L 8080:localhost:8080 <you>@<host>
# ---------------------
add_executable(web_gui
  src/web_gui.cpp
  src/board.cpp
  src/piece.cpp
  src/rook.cpp
  src/knight.cpp
  src/bishop.cpp
  src/queen.cpp
  src/king.cpp
  src/pawn.cpp
  src/print.cpp
  src/pieceMoves.cpp
  src/revealBoard.cpp
  src/game.cpp
)
target_include_directories(web_gui PRIVATE header)
