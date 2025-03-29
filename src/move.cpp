//===----------------------------------------------------------------------===//
//
// `Move` stores information about a pseudo-valid (valid format) move
// & allows easy access for some checks, and to generate hypothetical moves
// for threat checks. `MoveFactory` validates the move format prior to parsing
// so that we never attempt to parse invalidly formatted moves.
//
//===----------------------------------------------------------------------===//

#include "move.h"

#include <memory>
#include <string>

#include "basics.h"

// Move

// from string input
Move::Move(const std::string &input)
    : piece_char_(input[0]),
      captures_(input[3] == 'x'),
      from_(8 - (input[2] - '0'), input[1] - 'a'),
      to_(captures_ ? 8 - (input[5] - '0') : 8 - (input[4] - '0'), captures_ ? input[4] - 'a' : input[3] - 'a'),
      promotion_(input.size() > 6 && input[input.size() - 2] == '='),
      promote_to_(promotion_ ? input[input.size() - 1] : '\0') {}

// alternativ: moves zum ausprobieren aus Spielzustand generieren:
Move::Move(char piece_char, Field from, Field to, bool captures)
    : piece_char_(piece_char), captures_(captures), from_(from), to_(to), promotion_(false), promote_to_('\0') {}

char Move::piece_char() const { return piece_char_; }

bool Move::has_capture() const { return captures_; }

bool Move::is_promotion() const { return promotion_; }

char Move::promote_to() const { return promote_to_; }

Field Move::from() const { return from_; }

Field Move::to() const { return to_; }

bool Move::unobstructed(const Board &board) const {
  int o_row = from_.row;
  int o_col = from_.col;
  int d_row = to_.row;
  int d_col = to_.col;

  int row_step = (d_row == o_row) ? 0 : (d_row > o_row ? 1 : -1);
  int col_step = (d_col == o_col) ? 0 : (d_col > o_col ? 1 : -1);

  int row = o_row + row_step;
  int col = o_col + col_step;

  while (row != d_row || col != d_col) {
    if (board[row][col]) return false;
    row += row_step;
    col += col_step;
  }

  return true;  // no obstructions found
}

// Move factory

MoveFactory::MoveFactory() : move_format_("^[BKNPQRbknpqr][a-h][1-8]x?[a-h][1-8](=[BKNPQRbknpqr])?$") {}

bool MoveFactory::valid(const std::string &input) const { return std::regex_match(input, move_format_); }

std::shared_ptr<Move> MoveFactory::parse_move(const std::string &input) const { return std::make_shared<Move>(input); }