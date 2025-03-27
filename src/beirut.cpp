#include <iostream>
#include <chrono>
#include <thread>

#include "./beirut.h"

// color codes
#define WHITE     "\033[1;37m"
#define BLACK     "\033[1;30m"
#define GREEN     "\033[1;32m"
#define RESET     "\033[0m"
#define PINK_BG   "\033[45m"
#define CYAN_BG   "\033[46m"
#define YELLOW_BG "\033[1;43m"
#define RED_BG "\033[1;41m"
#define RESET_BG  "\033[49m"
#define CLEAR_SCREEN "\033[H\033[J"

Beirut::Beirut() : Game() {
  // show initial (or provided) board state, ask players to pick their bombers
  std::cout << "You are playing the 'Beirut' chess variant.\nBoth players will designate a piece as suicide bomber, who can detonate himself at any time."
            << "To detonate your carrier, type 'boom' on your turn. Detonation will destroy the bomb carrier and all pieces on adjacent fields.\n\n"
            << "NOTE: Since the identity of the bombers is secret, no check is called should the opposing king enter the explosion radius!";
};

void Beirut::get_bombers() const {
  // collect player inputs & give bombs to the pieces
}

void Beirut::boom(Player p) {
  // try to find player's bomb carrier:
  bool found = false;
  size_t brow, bcol;

  for (size_t i = 0; i < 8; ++i) {
    for (size_t j = 0; j < 8; ++j) {
      auto ptr = this->state_[i][j];

      if (!ptr) continue;

      if (ptr->carries_bomb() && ptr->owner() == p) {
        found = true;
        brow = i;
        bcol = j;
        break;
      }
    }
  }

  // if not found, print message to stdout and exit function.
  if (!found) {
    std::cout << "No bomb carrier for player " << (p == Player::White ? "white" : "black") << '\n';
    return;
  }

  // "detonate bomb"; delete 3x3 window around carrier:
  for (size_t i = brow - 1; i < brow + 1; ++i) {
    for (size_t j = bcol - 1; j < bcol + 1; ++j) {
      this->state_[i][j] = nullptr;
    }
  }

  // trigger explosion effect:
  this->explosion_effect(brow, bcol);
};

// basically print_board with different colors:
void Beirut::explosion_effect(size_t r, size_t c, bool char_view) const {
  const std::string cols = "    a  b  c  d  e  f  g  h   ";

  std::cout << CLEAR_SCREEN;
  std::cout << GREEN << cols << RESET << '\n';
  for (size_t i = 0; i < 8; ++i) {
    std::cout << GREEN << " " << 8 - i << RESET << ' ';

    for (size_t j = 0; j < 8; ++j) {
      const auto& ptr = this->state_[i][j];

      // if within radius make red, else make yellow
      std::cout << (((i >= r - 1) && (i <= r + 1)) && ((j >= c - 1) && (j <= c + 1)) ? RED_BG : YELLOW_BG);

      if (ptr)
        std::cout << " " << (ptr->owner() == Player::Black ? BLACK : WHITE) 
                  << (char_view ? std::string(1, ptr->to_char()) : ptr->unicode())
                  << " " << RESET_BG;
      else
        std::cout << "   " << RESET_BG;
    }

    std::cout << RESET << ' ' << GREEN << 8 - i << '\n';
  }

  // show for half a second then delegate back & show normal board again:
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  this->print_board();
};