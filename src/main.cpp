#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game.h"

bool char_mode = false;

int main(int argc, char** argv) {

  // Set gamemode & other setup:
  bool beirut = (argc > 1 && std::string(argv[1]) == "beirut");

  auto game = std::make_shared<Game>();

  if (beirut) {
    game->enable_beirut_mode();
    game->get_bomber(Player::White);
    game->get_bomber(Player::Black);
  }

  auto movemaker = std::make_unique<MoveFactory>();

  game->show(char_mode); // show initial state

  // main loop:
  std::string input;
  
  while(std::getline(std::cin, input)) {

    if (game->checkmate(game->to_move())) {
      std::cout << "Checkmate, game over!\n";
      break;
    }

    if (input == ":q") break;
    
    if (input == ":n") {
      game = std::make_shared<Game>();
      if (game->to_move() != Player::White) game->swap();

      if (beirut) {
        game->enable_beirut_mode();
        game->get_bomber(Player::White);
        game->get_bomber(Player::Black);
      }
      game->show(char_mode);
      continue;
    }

    if (input == ":u") {
      game->undo();
      game->swap();
      game->show(char_mode);
      continue;
    }

    if (input == ":t") {
      char_mode = !char_mode;
      game->show(char_mode);
      continue;
    }

    if (input.rfind(":m", 0) == 0 && input.length() > 2) {  
      std::string move_input = input.substr(2);
      game->print_moves(move_input, char_mode);
      continue;
    }

    if (input == "boom") {
      if (beirut) {
        game->boom(game->to_move());
        game->swap(); // no good, swaps also when boom is called without bomber...
        game->show();
        continue;
      }
    }

    // if not recognized as command we try to parse the input as move:

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

    game -> show(char_mode);
  }

  return EXIT_SUCCESS;
}
