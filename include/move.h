#pragma once

#include <memory>
#include <regex>
#include <string>

#include "./basics.h"

class Move {
  char piece_char_;
  bool captures_;
  Field from_, to_;
  bool promotion_;
  char promote_to_;

 public:
  explicit Move(const std::string &input);
  // alternate constructor to generate hypothetical moves:
  Move(char piece_char, Field from, Field to, bool captures);

  char piece_char() const;
  bool has_capture() const;
  bool is_promotion() const;
  char promote_to() const;
  Field from() const;
  Field to() const;
  bool unobstructed(const Board &board) const;
};

class MoveFactory {
  std::regex move_format_;

 public:
  MoveFactory();
  bool valid(const std::string &input) const;
  std::shared_ptr<Move> parse_move(const std::string &input) const;
};