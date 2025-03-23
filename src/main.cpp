#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game.h"

int main(void) {
  auto game = std::make_shared<Game>();
  auto movemaker = std::make_unique<MoveFactory>();

  game->print_board(); // show initial state

  std::string input;
  
  while(std::getline(std::cin, input)) {

    if (input == ":q") break;
    if (input == ":n") {
      game = std::make_shared<Game>();
      game->print_board();
      continue;
    }

    if (!movemaker->valid(input)) {
      std::cout << "Invalid format!\n" << "\033[43m" << "Input>" << "\033[49m";;
      continue;
    }

    auto move = movemaker->parse_move(input);

    if (!game->try_move(move)) {
      std::cout << "That move is not valid!\n" << "\033[43m" << "Input>" << "\033[49m";
      continue;
    }


    // All good, make move & swap players (next turn):
    game->make_move(move);
    game->swap();

    game -> print_board();

    if (game->checkmate(game->to_move())) {
      std::cout << "Checkmate, game over!\n";
    }
  }

  return EXIT_SUCCESS;
}
