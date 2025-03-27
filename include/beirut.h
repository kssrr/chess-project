#pragma once

#include "./game.h"

class Beirut : public Game {
 public:
  Beirut();
  void get_bombers() const;
  void boom(Player p);
  void explosion_effect(size_t r, size_t c, bool char_view=false) const;
};