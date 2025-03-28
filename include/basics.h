#pragma once

#include <memory>
#include <vector>

enum class Player { White, Black };

struct Field {
  int row, col;
  Field(int r, int c);
  Field();  // default constructor initializes to negative value
  bool valid() const;
};

class Piece;  // forward declare for board

typedef std::vector<std::vector<std::shared_ptr<class Piece>>> Board;