#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game.h"

int main(void) {
  auto game = std::make_shared<Game>();
  auto movemaker = std::make_unique<MoveFactory>();

  std::string input;
  
  while(std::getline(std::cin, input)) {

    if (!movemaker->valid(input)) {
      std::cout << "Invalid format!\n";
      continue;
    }

    auto move = movemaker->parse_move(input);

    if (!game->try_move(move)) {
      std::cout << "That move is not valid!\n";
      continue;
    }


    // All good, make move & swap players (next turn):
    game->make_move(move);
    game->swap();

    if (game->checkmate(game->to_move())) {
      std::cout << "Checkmate, game over!\n";
    }
  }

  return EXIT_SUCCESS;
}
