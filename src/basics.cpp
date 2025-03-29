//===----------------------------------------------------------------------===//
//
// This only implements properties of the `Field`-class. The `basics.h` header
// also holds some other additional basic constructs (players and boards).
//
//===----------------------------------------------------------------------===//

#include "basics.h"

// Field

Field::Field(int r, int c) : row(r), col(c) {}

Field::Field() : row(-1), col(-1) {}

bool Field::valid() const { return (row != -1) && (col != -1); }