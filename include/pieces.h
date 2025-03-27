#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <unordered_map>

#include "./basics.h"
#include "./move.h"

class Piece {
 protected:
  Player player_;
  char rep_;
  uint32_t unicode_;
  bool carries_bomb_; // for beirut variant

 public:
  Piece(Player p, char c);
  char to_char() const;
  std::string unicode() const;
  Player owner() const;
  virtual bool valid(std::shared_ptr<Move> move, const Board& board) const = 0;
  // for beirut variant:
  bool carries_bomb() const;
  void give_bomb();
};

class Bishop : public Piece {
 public:
  explicit Bishop(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class King : public Piece {
 public:
  explicit King(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class Knight : public Piece {
 public:
  explicit Knight(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class Pawn : public Piece {
 public:
  explicit Pawn(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class Queen : public Piece {
 public:
  explicit Queen(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class Rook : public Piece {
 public:
  explicit Rook(Player p);
  bool valid(std::shared_ptr<Move> move, const Board& board) const override;
};

class PieceFactory {
  std::unordered_map<char, std::function<std::shared_ptr<Piece>(Player)>> pieces_;

 public:
  PieceFactory();
  std::shared_ptr<Piece> make_piece(char c) const;
};