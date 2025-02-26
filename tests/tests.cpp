#include "basics.h"
#include "move.h"
#include "pieces.h"
#include "game.h"

#include <gtest/gtest.h>

TEST(ChessTests, GameInitTest) {
  auto game = std::make_shared<Game>(); // non-file init
}

// Testing checkmate recognition

TEST(ChessTests, CheckmateTest) {
  auto game = std::make_shared<Game>("r  r  k   q bpp    p   p ppn     P BP   P     Q     RPPPR     K ");
  auto movemaker = std::make_shared<MoveFactory>();
  std::string move_inp = "Qg3xg7";
  ASSERT_TRUE(movemaker->valid(move_inp)) << "In CheckmateTest: valid move format not recognized (Player order?)";
  auto move = movemaker->parse_move(move_inp);

  ASSERT_TRUE(game->try_move(move)) << "In CheckmateTest: substantively valid move not recognized";
  game->make_move(move);
  game->swap();
  ASSERT_TRUE(game->checkmate(game->to_move())) << "In CheckmateTest: checkmate not recognized";
}

/**********************/
/* Runnning the tests */
/**********************/

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}