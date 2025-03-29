/* The `Game` class is the heart and soul (and basically half of the code)
   of this program. It can start a new game, or read board status from a file.
   The actual game logic is handled in this class. */

// TODO: adapt checkmate to understand checkmate by blowing up
// for example, in `boom` check for the king in the radius &
// if he blows up invalidate the king's position (-1, -1)?
// Then check if a kings position is valid in check & checkmate?

#include "game.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <vector>

// color codes
#define WHITE "\033[1;37m"
#define BLACK "\033[1;30m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"
#define PINK_BG "\033[45m"
#define CYAN_BG "\033[46m"
#define YELLOW_BG "\033[1;43m"
#define RED_BG "\033[1;41m"
#define RESET_BG "\033[49m"

#define CLEAR_SCREEN "\033[H\033[J"

// initial board state if none is provided:
Board Game::init_board() const {
  Board board(8, std::vector<std::shared_ptr<Piece>>(8, nullptr));
  board[0] = {std::make_shared<Rook>(Player::Black),   std::make_shared<Knight>(Player::Black),
              std::make_shared<Bishop>(Player::Black), std::make_shared<Queen>(Player::Black),
              std::make_shared<King>(Player::Black),   std::make_shared<Bishop>(Player::Black),
              std::make_shared<Knight>(Player::Black), std::make_shared<Rook>(Player::Black)};

  for (std::size_t i = 0; i < board[1].size(); i++) {
    board[1][i] = std::make_shared<Pawn>(Player::Black);
  }

  for (std::size_t i = 0; i < board[6].size(); i++) {
    board[6][i] = std::make_shared<Pawn>(Player::White);
  }

  board[7] = {std::make_shared<Rook>(Player::White),   std::make_shared<Knight>(Player::White),
              std::make_shared<Bishop>(Player::White), std::make_shared<Queen>(Player::White),
              std::make_shared<King>(Player::White),   std::make_shared<Bishop>(Player::White),
              std::make_shared<Knight>(Player::White), std::make_shared<Rook>(Player::White)};

  return board;
}

/* Next to initializing the board we also keep track of the kings' positions.
   This means we won't have to look for them later if we test check & checkmate.
 */
Game::Game() : state_(init_board()) {}

// Initializing a game from a provided board state
Game::Game(const std::string &input) {
  Board board(8, std::vector<std::shared_ptr<Piece>>(8, nullptr));
  PieceFactory piecemaker;

  for (int i = 0; i < 64; ++i) {
    char c = input[i];
    int row = i / 8;
    int col = i % 8;

    if (c != ' ') board[row][col] = piecemaker.make_piece(c);
  }

  state_ = board;
  current_player_ = Player::White;  // white always starts, even when reading from file
}

Board Game::board() const { return state_; }

void Game::print_board(bool char_view) const {
  const std::string cols = "    a  b  c  d  e  f  g  h   ";

  std::cout << CLEAR_SCREEN;
  std::cout << GREEN << cols << RESET << '\n';
  for (size_t i = 0; i < 8; ++i) {
    std::cout << GREEN << " " << 8 - i << RESET << ' ';

    for (size_t j = 0; j < 8; ++j) {
      const auto &ptr = state_[i][j];

      std::cout << ((i + j) % 2 == 0 ? CYAN_BG : PINK_BG);

      if (ptr)
        std::cout << " " << (ptr->owner() == Player::Black ? BLACK : WHITE)
                  << (char_view ? std::string(1, ptr->to_char()) : ptr->unicode()) << " " << RESET_BG;
      else
        std::cout << "   " << RESET_BG;
    }

    std::cout << RESET << ' ' << GREEN << 8 - i << '\n';
  }
  std::cout << GREEN << cols << RESET << '\n';
}

void Game::show(bool char_view) const {
  // board, player status, commands & input prompt:
  print_board(char_view);
  std::cout << (in_check(to_move()) ? "CHECK! " : "")
            << (to_move() == Player::White ? "White" : "Black") << "'s turn.\n"
            << "Commands: (:n)ew game (:u)ndo (:q)uit (:m)oves (:t)oggle character mode\n"
            << "\033[43m" << "Input>" << RESET_BG;
}

Player Game::to_move() const { return current_player_; }

void Game::swap() {
  current_player_ == Player::White ? current_player_ = Player::Black
                                         : current_player_ = Player::White;
}

void Game::make_move(std::shared_ptr<Move> move) {
  history_.push(state_);

  Field from = move->from();
  Field to = move->to();

  state_[to.row][to.col] = state_[from.row][from.col];
  state_[from.row][from.col] = nullptr;

  // handle promotion:
  if (move->is_promotion()) {
    auto piecemaker = std::make_unique<PieceFactory>();
    state_[to.row][to.col] = piecemaker->make_piece(move->promote_to());
  }
}

void Game::undo() {
  auto prev_state = history_.top();
  history_.pop();

  state_ = prev_state;
}

bool Game::substantively_valid(std::shared_ptr<Move> move, bool threat_check = false) const {
  /* The threat_check flag overrides ownership tests, so we can
  check whether a king is in check regardless of whose turn it is. */
  Field from = move->from();
  Field to = move->to();
  char ref_piece = move->piece_char();

  auto piece_at_start = state_[from.row][from.col];
  auto piece_at_dest = state_[to.row][to.col];

  if (!piece_at_start)
    return false;  // no piece at starting loc (also prevents nullptr deref in
                   // later checks)

  if (!threat_check && (current_player_ != piece_at_start->owner()))
    return false;  // piece does not belong to moving player

  if (ref_piece != piece_at_start->to_char()) return false;  // referenced piece not at starting loc.

  if (move->has_capture() && !piece_at_dest) return false;  // marked as capture but no piece at dest

  if (!threat_check && (move->has_capture() && piece_at_dest->owner() == current_player_))
    return false;  // piece to capture belongs to moving player

  if (!piece_at_start->valid(move, state_)) return false;  // piece cannot move like this

  // Check pawn promotion: (this should ideally be done in Pawn::valid)
  if (move->is_promotion()) {
    if (ref_piece != 'P' && ref_piece != 'p') return false;  // only pawns can be promoted

    int promotion_row = (piece_at_start->owner() == Player::White) ? 0 : 7;
    if (to.row != promotion_row) return false;  // promotion move has to end up in opposing back row

    // promotion cannot change the owner of the piece:
    Player owner_after_promotion = std::isupper(move->promote_to()) ? Player::White : Player::Black;
    if (piece_at_start->owner() != owner_after_promotion) return false;

    if (move->promote_to() == move->piece_char()) return false;  // piece has to be promoted (cannot remain itself)

    if (move->promote_to() == 'k' || move->promote_to() == 'K') return false;  // cannot promote to become a king
  }

  return true;  // If none of the above conditions failed, the move is valid
}

Field Game::kingpos(Player p) const {
  char c = (p == Player::White ? 'K' : 'k');

  // find the king
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      auto ptr = state_[i][j];

      if (!ptr) continue;

      if (ptr->to_char() == c) {
        return Field(i, j);
      }
    }
  }

  return Field();  // return default (invalid) field by default
}

bool Game::in_check(Player p) const {
  Field king_field = kingpos(p);

  // find opposing pieces & see if they can perform a valid capture move towards
  // the king:
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      if (state_[row][col]) {
        if (state_[row][col]->owner() != p) {
          auto current_piece = state_[row][col];
          auto king_attack = std::make_shared<Move>(current_piece->to_char(), Field(row, col), king_field, true);

          // if piece can perform valid capture move on king, king is in check
          // (mark as threat_check):
          if (substantively_valid(king_attack, true)) return true;
        }
      }
    }
  }

  return false;  // we could not find any piece that threatens the king
}

// Move probieren & zurücksetzen (kann benutzt werden um
// zu prüfen ob der Zug den aktuellen Spieler Schach setzt)
bool Game::try_move(std::shared_ptr<Move> move) {
  if (!substantively_valid(move, false)) return false;

  make_move(move);

  if (in_check(current_player_)) {
    undo();
    return false;
  }

  undo();
  return true;
}

// Dumme brute-force Lösung für Schachmatt
bool Game::checkmate(Player p) {
  /*
  This first check is neccessitated by the Beirut-variant, where
  the king might be "blown up" without prior check, and thus may
  simply "disappear" from the game's view. If the kingfield is not valid
  (empty field returned by kingpos = could not find king) the player has
  lost their king in an explosion.
  */
  Field king = kingpos(p);
  if (!king.valid()) return true;

  // and then from here we proceed "normally"
  if (!in_check(p)) return false;  // cannot be checkmate if not in check

  // alle Figuren des Spielers finden:
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      auto piece = state_[row][col];
      if (!piece || piece->owner() != p) continue;

      // alle möglichen moves generieren:
      for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
          if (r == row && c == col) continue;  // Skip the same square

          auto move = std::make_shared<Move>(piece->to_char(), Field(row, col), Field(r, c), state_[r][c] != nullptr);

          // einfach ausprobieren & dann schauen ob Spieler noch im Schach:
          if (try_move(move)) {
            return false;  // At least one move is possible → Not checkmate
          }
        }
      }
    }
  }

  return true;  // keine erlaubten moves
}

void Game::print_moves(const std::string &input, const bool char_view) {
  const std::string cols = "    a  b  c  d  e  f  g  h   ";

  char piece_char = input[0];
  Field from(8 - (input[2] - '0'), input[1] - 'a');

  std::cout << CLEAR_SCREEN;
  std::cout << GREEN << cols << RESET << '\n';
  for (size_t i = 0; i < 8; ++i) {
    std::cout << GREEN << " " << 8 - i << RESET << ' ';

    for (size_t j = 0; j < 8; ++j) {
      const auto &ptr = state_[i][j];
      bool occupied = ptr ? true : false;
      Field to(i, j);
      auto move = std::make_shared<Move>(piece_char, from, to, occupied);
      bool valid = try_move(move);

      if (valid)
        std::cout << YELLOW_BG;
      else
        std::cout << ((i + j) % 2 == 0 ? CYAN_BG : PINK_BG);

      if (ptr)
        std::cout << " " << (ptr->owner() == Player::Black ? BLACK : WHITE)
                  << (char_view ? std::string(1, ptr->to_char()) : ptr->unicode()) << " " << RESET_BG;
      else
        std::cout << "   " << RESET_BG;
    }

    std::cout << RESET << ' ' << GREEN << 8 - i << '\n';
  }
  std::cout << GREEN << cols << RESET << '\n';
  std::cout << (in_check(to_move()) ? "CHECK! " : "")
            << (to_move() == Player::White ? "White" : "Black") << "'s turn.\n"
            << "Commands: (:n)ew game (:u)ndo (:q)uit (:m)oves (:t)oggle character mode\n"
            << "\033[43m" << "Input>" << RESET_BG;
}

// Beirut-mode specifics

bool Game::beirut_mode() const { return beirut_mode_; }

void Game::enable_beirut_mode() { beirut_mode_ = true; }

void Game::get_bomber(Player p, bool char_view) const {
  // collect player inputs & give bombs to the pieces
  print_board(char_view);

  std::regex valid_regex(p == Player::White ? "^[BNPQR][a-h][1-2]$" : "^[bnpqr][a-h][7-8]$");  // no king allowed
  std::string input;

  std::cout << (p == Player::White ? "White" : "Black") << "'s suicide bomber:>";

  while (std::getline(std::cin, input)) {
    if (!std::regex_match(input, valid_regex)) {
      std::cout << "Invalid format; enter a piece belonging to you followed by "
                   "a field.\n>";
      continue;
    }

    // here, deparse the input into piece and field, check if the piece is at
    // the field. If not, get new input. If it is, give bomb to piece.
    char piece_char = input[0];
    Field location(8 - (input[2] - '0'), input[1] - 'a');

    auto ptr = state_[location.row][location.col];

    if (!ptr) {
      std::cout << "No piece at that location, try again\n>";
      continue;
    }

    if (ptr->to_char() != piece_char) {
      std::cout << "That is not the piece at the location, that piece is " << ptr->to_char() << ". Try again\n>";
      continue;
    }

    ptr->give_bomb();
    break;
  }
}

bool Game::boom(Player p) {
  // try to find player's bomb carrier:
  bool found = false;
  int brow, bcol;

  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      auto ptr = state_[i][j];

      if (!ptr) continue;

      if (ptr->carries_bomb() && ptr->owner() == p) {
        found = true;
        brow = i;
        bcol = j;
        break;
      }
    }
  }

  // if not found, print message to stdout and exit function.
  if (!found) {
    std::cout << "No bomb carrier for player " << (p == Player::White ? "white" : "black") << '\n';
    return found;
  }

  history_.push(state_);

  // "detonate bomb"; delete 3x3 window around carrier:
  for (int i = std::max(0, brow - 1); i <= std::min(7, brow + 1); ++i) {
    for (int j = std::max(0, bcol - 1); j <= std::min(7, bcol + 1); ++j) {
      state_[i][j] = nullptr;
    }
  }

  // trigger explosion effect:
  explosion_effect(brow, bcol);
  return found;
};

// basically print_board with different colors:
void Game::explosion_effect(int r, int c, bool char_view) const {
  const std::string cols = "    a  b  c  d  e  f  g  h   ";

  std::cout << CLEAR_SCREEN;
  std::cout << GREEN << cols << RESET << '\n';
  for (int i = 0; i < 8; ++i) {
    std::cout << GREEN << " " << 8 - i << RESET << ' ';

    for (int j = 0; j < 8; ++j) {
      const auto &ptr = state_[i][j];

      // if within radius make red, else make yellow
      // TODO: there is a small bug here, because r+/-1 or c +/- 1 might be out
      // of bounds, so we get no red
      std::cout << (((i >= std::max(0, r - 1)) && (i <= std::min(7, r + 1))) &&
                            ((j >= std::max(0, c - 1)) && (j <= std::min(7, c + 1)))
                        ? RED_BG
                        : YELLOW_BG);

      if (ptr)
        std::cout << " " << WHITE << (char_view ? std::string(1, ptr->to_char()) : ptr->unicode()) << " " << RESET_BG;
      else
        std::cout << "   " << RESET_BG;
    }

    std::cout << RESET << ' ' << GREEN << 8 - i << '\n';
  }

  // show for half a second then delegate back & show normal board again:
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  print_board();
};