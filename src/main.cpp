#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game.h"

int main(int argc, char** argv) {
  (void)argc;
  std::ifstream input_file(argv[1]);
  std::string line;

  std::shared_ptr<Game> game = nullptr;
  auto movemaker = std::make_unique<MoveFactory>();

  while (std::getline(input_file, line)) {
    char identifier = line[0];
    std::string content = line.substr(1);

    if (identifier == 'B') {
      game = std::make_shared<Game>(content);
      std::cout << (game->checkmate(Player::White) ? "yes\n" : "no\n");
    }

    if (identifier == 'F') {
      // Should be owner's turn for this check, if not make it so:
      Player owner = std::islower(content[0]) ? Player::Black : Player::White;

      if (owner != game->to_move()) game->swap();

      game->print_moves(content);
      continue;
    }

    if (identifier == 'M') {
      if (!movemaker->valid(content)) {
        std::cout << "invalid\n";
        continue;
      }

      auto move = movemaker->parse_move(content);

      /* Try the move. If there are issues (invalid)
       or leaves player in check), abort. */
      if (!game->try_move(move)) {
        std::cout << "invalid\n";
        continue;
      }

      // All good, make move & swap players (next turn):
      game->make_move(move);
      game->swap();

      std::cout << (game->checkmate(game->to_move()) ? "yes\n" : "no\n");
    }
  }

  return EXIT_SUCCESS;
}
