#include <gtest/gtest.h>

#include "basics.h"
#include "game.h"
#include "move.h"
#include "pieces.h"

// Game initialization
// See if initializing w/ defaults or provided state causes issues:

TEST(ChessTests, GameInitTest) {
  auto default_init = std::make_unique<Game>();
  auto state_provided = std::make_unique<Game>("rnbqkbnrpppppppp                                PPPPPPPPRNBQKBNR");
}

// Input tests
// See if the move factory detects invalid inputs

TEST(ChessTests, MoveFormatTest) {
  auto movemaker = std::make_unique<MoveFactory>();

  std::vector<std::string> invalid_inputs = {
      "AAAAAAAAAAAH", "Pe2xxxe4", "Pe27e2", "", "\033", "他们甚至无法找到的炸弹客 直到他自己的兄弟把他"};

  for (const auto &input : invalid_inputs) {
    ASSERT_FALSE(movemaker->valid(input)) << "In MoveFormatTest: invalid format not recognized";
  }

  // recognize normal move, capture, and promotion?
  std::vector<std::string> valid_inputs = {"Pe2e4", "Nf3xd4", "Pd7d8=P"};

  for (const auto &input : valid_inputs) {
    ASSERT_TRUE(movemaker->valid(input)) << "In MoveFormatTest: valid format not recognized";
  }
}

// Testing move validity (piece-specific move sets & obstruction)

TEST(ChessTests, MoveTests) {
  // try to make some invalid moves with pieces (the format is always valid)
  auto game = std::make_unique<Game>("rnbqkbnrpppp ppp            p       P           P PP PPPRNBQKBNR");
  auto movemaker = std::make_unique<MoveFactory>();

  std::vector<std::string> invalid_moves = {
      "Bf1h3",  // theoretically valid but obstructed
      "Ra1b2",  // rook cannot move diagonally
      "Nb1b2",  // knight jumps, cannot move one square
      "Qd1d3",  // queen cannot jump (also obstructed)
      "Ke1e3"   // king cannot move two squares
  };

  for (const auto &input : invalid_moves) {
    auto move = movemaker->parse_move(input);
    ASSERT_FALSE(game->try_move(move)) << "In MoveTests: invalid move allowed";
  }
}

// Checkmate recognition

TEST(ChessTests, CheckmateTest) {
  // checkmate after capture:
  auto game = std::make_unique<Game>("r  r  k   q bpp    p   p ppn     P BP   P     Q     RPPPR     K ");
  auto movemaker = std::make_unique<MoveFactory>();
  std::string move_inp = "Qg3xg7";
  auto move = movemaker->parse_move(move_inp);
  game->make_move(move);
  game->swap();
  ASSERT_TRUE(game->checkmate(game->to_move())) << "In CheckmateTest: checkmate not recognized";

  // checkmate after explosion (no king on board):
  auto game2 = std::make_unique<Game>("r  r  k   q bpp    p   p ppn     P BP   P     Q     RPPPR       ");
  ASSERT_TRUE(game2->checkmate(Player::White)) << "In CheckmateTest: checkmate by missing king not recognized";
}

// Check recognition
// See if the game recognizes when a player is in check, and whether it can tell
// check from checkmate

TEST(ChessTests, CheckTest) {
  // black in check by pawn, king can evade:
  auto game = std::make_unique<Game>("rn  kbnrpppPpppp                                PPP PPPPRNBQKBNR");
  game->swap();
  ASSERT_TRUE(game->in_check(game->to_move())) << "In CheckTest: check not recognized";
  ASSERT_FALSE(game->checkmate(game->to_move())) << "In CheckTest: check mistaken for checkmate (1)";

  // white in check by rook, king can evade
  auto game2 = std::make_unique<Game>("     rk  p   ppppq   b                   P Q N  P    PPP   r K  ");
  ASSERT_TRUE(game2->in_check(game2->to_move())) << "In CheckTest: check not recognized";
  ASSERT_FALSE(game2->checkmate(game2->to_move())) << "In CheckTest: check mistaken for checkmate (2)";
}

// Pawn promotion

TEST(ChessTests, PawnPromotionTest) {
  // Cannot promote to certain pieces (pawn, king)
  auto game = std::make_unique<Game>("rn  kbnrpppPpppp                                PPP PPPPRNBQKBNR");
  auto movemaker = std::make_unique<MoveFactory>();
  std::string move_inp = "Pd7d8=P";
  auto move = movemaker->parse_move(move_inp);
  ASSERT_FALSE(game->try_move(move)) << "In PawnPromotionTest: invalid promotion allowed (cannot promote to "
                                        "pawn)";

  std::string promote_to_king = "Pd7d8=K";
  auto move_king = movemaker->parse_move(promote_to_king);
  ASSERT_FALSE(game->try_move(move_king)) << "In PawnPromotionTest: invalid promotion allowed (cannot promote to "
                                             "king)";

  // cross-check some valid promotion:
  std::string valid_prom = "Pd7d8=Q";
  auto valid_prom_move = movemaker->parse_move(valid_prom);
  ASSERT_TRUE(game->try_move(valid_prom_move)) << "In PawnPromotionTest: valid promotion not recognized";
}

// just simulate a few rounds of playing w/ some captures
// see if anything goes wrong:

TEST(ChessTests, GameplayTest) {
  auto game = std::make_unique<Game>();
  auto movemaker = std::make_unique<MoveFactory>();

  std::vector<std::string> moves = {"Pe2e4", "pe7e5", "Ng1f3", "bf8c5", "Pd2d4", "bc5xd4", "Nf3xd4", "pe5xd4"};

  for (const auto &input : moves) {
    auto move = movemaker->parse_move(input);
    ASSERT_TRUE(game->try_move(move)) << "In GameplayTest: valid move not recognized";
    game->make_move(move);
    game->swap();
  }
}

/**********************/
/* Runnning the tests */
/**********************/

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
