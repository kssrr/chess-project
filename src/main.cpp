//===----------------------------------------------------------------------===//
//
// This is the main function that runs the actual game loop. The game loop
// is layed out in `play` and reads from `std::cin`. Basically, we first check
// whether an input matches a known command. If not, we try to validate &
// parse it as move. If not, we keep trying to get a valid/recognized input
// from the player. After a valid move has been made, it's the other player's
// turn.
//
//===----------------------------------------------------------------------===//

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game.h"

void show_prompt() { std::cout << "\033[43m" << "Input>" << "\033[49m"; }

void play(std::shared_ptr<Game> game, std::shared_ptr<MoveFactory> movemaker, bool char_mode) {
  bool beirut = game->beirut_mode();
  std::string input;

  while (std::getline(std::cin, input)) {
    // First check if the input matches any command:

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
      if (!beirut) {
        show_prompt();
        continue;
      }

      bool bomber_found = game->boom(game->to_move());

      if (!bomber_found) {
        show_prompt();
        continue;
      }

      // player could accidentally checkmate themselves with bomb:
      if (game->checkmate(game->to_move())) {
        std::cout << "You blew up your own king you retard\n";
        break;
      }

      game->swap();

      if (game->checkmate(game->to_move())) {
        std::cout << "Checkmate, game over\n";
        break;
      }

      game->show();
      continue;
    }

    // if not recognized as command we try to parse the input as move:

    if (!movemaker->valid(input)) {
      std::cout << "Invalid format!\n";
      show_prompt();
      continue;
    }

    auto move = movemaker->parse_move(input);

    if (!game->try_move(move)) {
      std::cout << "That move is not valid!\n";
      show_prompt();
      continue;
    }

    // All good, make move & swap players (next turn):

    game->make_move(move);
    game->swap();

    if (game->checkmate(game->to_move())) {
      std::cout << "Checkmate, game over\n";
      break;
    };

    game->show(char_mode);
  }
}

int main(int argc, char **argv) {
  // Set gamemode & other setup:
  bool char_mode = false;  // by default, try to show board with unicode piece chars
  bool beirut = (argc > 1 && std::string(argv[1]) == "beirut");
  auto game = std::make_shared<Game>();

  if (beirut) {
    game->enable_beirut_mode();
    game->get_bomber(Player::White);
    game->get_bomber(Player::Black);
  }

  auto movemaker = std::make_shared<MoveFactory>();  // validates move inputs

  game->show(char_mode);  // show initial state

  // main loop:
  try {
    play(game, movemaker, char_mode);
  } catch (...) {
    std::cout << "An issue has occurred, terminating...\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
