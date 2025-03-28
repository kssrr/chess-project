#pragma once

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>

#include "./basics.h"
#include "./move.h"
#include "./pieces.h"

class Game {
  // a game state includes the board state, and the positions of the kings:
  using GameState = std::pair<Board, std::map<Player, Field>>;

  Board state_;
  std::stack<GameState> history_;
  Player current_player_;
  std::map<Player, Field> kings_;  // keeping track of the kings' positions
  bool beirut_mode_;

 public:
  Game();
  virtual ~Game() = default; // needed to make polymorphic (?)
  explicit Game(const std::string& input);
  Board init_board() const;
  void print_board(bool char_view=false) const;
  void show(bool char_view=false) const;
  Board board() const;
  Player to_move() const;  // returns current player
  void swap();
  void make_move(std::shared_ptr<Move> move);
  void undo();
  bool substantively_valid(std::shared_ptr<Move> move, bool threat_check) const;

  Field kingpos(Player p) const;
  bool in_check(Player p) const;
  /* `checkmate` and `try_move` are technically const,
  they only make temporary modifications which they revert
  after being called, but we cannot mark them const since
  they need to call non-const members like `make_move`. */
  bool checkmate(Player p);
  bool try_move(std::shared_ptr<Move> move);
  void print_moves(const std::string& input, const bool char_view=false); // same here

  // Beirut-variant specific:
  bool beirut_mode() const;
  void enable_beirut_mode();
  void get_bomber(Player p, bool char_view=false) const;
  // ^ view mode necessary because we show the board for picking a bomber
  void boom(Player p);
  void explosion_effect(size_t r, size_t c, bool char_view=false) const;
};