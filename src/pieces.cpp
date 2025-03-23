/* All pieces inherit from the `Piece`-class and only differ in their character
   representation & move validator (as every piece moves differently). The
   `PieceFactory` can create instances of pieces (or rather pointers to instances
   of pieces) from their character representation (e.g. when parsing a move input). */

#include "pieces.h"

#include <memory>

#include "basics.h"
#include "move.h"

Piece::Piece(Player p, char c) : player_(p) {
  this->rep_ = (this->player_ == Player::White) ? std::toupper(c) : std::tolower(c);
  this->unicode_ = 0;
}

char Piece::to_char() const { return this->rep_; }

Player Piece::owner() const { return this->player_; }

Bishop::Bishop(Player p) : Piece(p, 'B') { unicode_ = p == Player::White ? 0x2657 : 0x265D; }

bool Bishop::valid(std::shared_ptr<Move> move, const Board& board) const {
  Field from = move->from();
  Field to = move->to();

  int dx = std::abs(to.col - from.col);
  int dy = std::abs(to.row - from.row);

  return (dx == dy) && move->unobstructed(board);  // diagonal move (same vertical and horizontal diff)
}

King::King(Player p) : Piece(p, 'K') { unicode_ = p == Player::White ? 0x2654 : 0x265A; }

bool King::valid(std::shared_ptr<Move> move, const Board& board) const {
  (void)board;  // unused

  Field from = move->from();
  Field to = move->to();

  return (std::abs(to.col - from.col) <= 1 && std::abs(to.row - from.row) <= 1);
}

Knight::Knight(Player p) : Piece(p, 'N') { unicode_ = p == Player::White ? 0x2658 : 0x265E; }

bool Knight::valid(std::shared_ptr<Move> move, const Board& board) const {
  (void)board;  // unused

  Field from = move->from();
  Field to = move->to();

  int dx = std::abs(to.col - from.col);
  int dy = std::abs(to.row - from.row);

  return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
}

Pawn::Pawn(Player p) : Piece(p, 'P') { unicode_ = p == Player::White ? 0x2659 : 0x265F; }

bool Pawn::valid(std::shared_ptr<Move> move, const Board& board) const {
  int direction = (this->owner() == Player::White) ? -1 : 1;

  Field from = move->from();
  Field to = move->to();

  int dx = to.col - from.col;
  int dy = to.row - from.row;

  // single move forward:
  if (dx == 0 && dy == direction && !board[to.row][to.col]) return true;

  // double move forward (only from starting position):
  int start_row = (this->owner() == Player::White) ? 6 : 1;
  if (dx == 0 && dy == 2 * direction && from.row == start_row && !board[to.row][to.col] && move->unobstructed(board))
    return true;

  // capture (diagonally):
  if (std::abs(dx) == 1 && dy == direction && move->has_capture() && board[to.row][to.col]) return true;

  return false;
}

Queen::Queen(Player p) : Piece(p, 'Q') { unicode_ = p == Player::White ? 0x2655 : 0x265B; }

bool Queen::valid(std::shared_ptr<Move> move, const Board& board) const {
  Field from = move->from();
  Field to = move->to();

  int dx = std::abs(to.col - from.col);
  int dy = std::abs(to.row - from.row);

  return ((dx == dy) || (dx == 0 || dy == 0)) && move->unobstructed(board);
}

Rook::Rook(Player p) : Piece(p, 'R') { unicode_ = p == Player::White ? 0x2656 : 0x265C; }

bool Rook::valid(std::shared_ptr<Move> move, const Board& board) const {
  Field from = move->from();
  Field to = move->to();

  int dx = std::abs(to.col - from.col);
  int dy = std::abs(to.row - from.row);

  return (dx == 0 || dy == 0) && move->unobstructed(board);
}

// Factories

PieceFactory::PieceFactory() {
  pieces_ = {
      {'b', [](Player p) { return std::make_shared<Bishop>(p); }},
      {'k', [](Player p) { return std::make_shared<King>(p); }},
      {'q', [](Player p) { return std::make_shared<Queen>(p); }},
      {'r', [](Player p) { return std::make_shared<Rook>(p); }},
      {'n', [](Player p) { return std::make_shared<Knight>(p); }},
      {'p', [](Player p) { return std::make_shared<Pawn>(p); }},
  };
}

std::shared_ptr<Piece> PieceFactory::make_piece(char c) const {
  Player player = std::isupper(c) ? Player::White : Player::Black;
  char lower = std::tolower(c);

  auto match = pieces_.find(lower);
  return match->second(player);
}