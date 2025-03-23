/* The `Game` class is the heart and soul (and basically half of the code)
   of this program. It can start a new game, or read board status from a file.
   The actual game logic is handled in this class. */

#include "game.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// color codes
#define WHITE    "\033[1;37m"
#define BLACK    "\033[1;30m"
#define GREEN    "\033[1;32m"
#define RESET    "\033[0m"
#define PINK_BG  "\033[45m"
#define CYAN_BG  "\033[46m"
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
   This means we won't have to look for them later if we test check & checkmate. */
Game::Game() : state_(init_board()) { kings_ = {{Player::White, {7, 4}}, {Player::Black, {0, 4}}}; }

// Initializing a game from a provided board state
Game::Game(const std::string& input) {
  Board board(8, std::vector<std::shared_ptr<Piece>>(8, nullptr));
  PieceFactory piecemaker;

  for (int i = 0; i < 64; ++i) {
    char c = input[i];
    int row = i / 8;
    int col = i % 8;

    if (c != ' ') {
      board[row][col] = piecemaker.make_piece(c);

      if (c == 'k') kings_[Player::Black] = Field(row, col);
      if (c == 'K') kings_[Player::White] = Field(row, col);
    }
  }

  state_ = board;
  current_player_ = Player::White;  // white always starts, even when reading from file
}

Board Game::board() const { return this->state_; }

void Game::print_board(bool char_view) const {
  const std::string cols = "    a  b  c  d  e  f  g  h   ";

  std::cout << CLEAR_SCREEN;
  std::cout << GREEN << cols << RESET << '\n';
  for (size_t i = 0; i < 8; ++i) {
    std::cout << GREEN << " " << 8 - i << RESET << ' ';

    for (size_t j = 0; j < 8; ++j) {
      const auto& ptr = this->state_[i][j];

      std::cout << ((i + j) % 2 == 0 ? CYAN_BG : PINK_BG);

      if (ptr)
        std::cout << " " << (ptr->owner() == Player::Black ? BLACK : WHITE) 
                  << (char_view ? std::string(1, ptr->to_char()) : ptr->unicode())
                  << " " << RESET_BG;
      else
        std::cout << "   " << RESET_BG;
    }

    std::cout << RESET << ' ' << GREEN << 8 - i << '\n';
  }
  std::cout << GREEN << cols << RESET << '\n';
  std::cout << (this->to_move() == Player::White ? "White" : "Black") << "'s turn.\n";
  std::cout << "Commands: (:n)ew game (:u)ndo (:q)uit (:t)oggle character mode\n";
  std::cout << "\033[43m" << "Input>" << RESET_BG;
}

Player Game::to_move() const { return this->current_player_; }

void Game::swap() {
  this->current_player_ == Player::White ? this->current_player_ = Player::Black
                                         : this->current_player_ = Player::White;
}

void Game::make_move(std::shared_ptr<Move> move) {
  history_.push({state_, kings_});

  Field from = move->from();
  Field to = move->to();

  this->state_[to.row][to.col] = this->state_[from.row][from.col];
  this->state_[from.row][from.col] = nullptr;

  // handle promotion:
  if (move->is_promotion()) {
    auto piecemaker = std::make_unique<PieceFactory>();
    this->state_[to.row][to.col] = piecemaker->make_piece(move->promote_to());
  }

  // keep track of the king if he moves:
  if (std::tolower(move->piece_char()) == 'k') {
    Player owner = std::isupper(move->piece_char()) ? Player::White : Player::Black;
    kings_[owner] = {to.row, to.col};
  }
}

void Game::undo() {
  auto [prev_state, prev_kings] = history_.top();
  history_.pop();

  state_ = prev_state;
  kings_ = prev_kings;
  // swap();
}

bool Game::substantively_valid(std::shared_ptr<Move> move, bool threat_check = false) const {
  /* The threat_check flag overrides ownership tests, so we can
  check whether a king is in check regardless of whose turn it is. */
  Field from = move->from();
  Field to = move->to();
  char ref_piece = move->piece_char();

  auto piece_at_start = state_[from.row][from.col];
  auto piece_at_dest = state_[to.row][to.col];

  if (!piece_at_start) return false;  // no piece at starting loc (also prevents nullptr deref in later checks)

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

Field Game::kingpos(Player p) const { return kings_.at(p); }

bool Game::in_check(Player p) const {
  Field king_field = kingpos(p);

  // find opposing pieces & see if they can perform a valid capture move towards the king:
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      if (state_[row][col]) {
        if (state_[row][col]->owner() != p) {
          auto current_piece = state_[row][col];
          auto king_attack = std::make_shared<Move>(current_piece->to_char(), Field(row, col), king_field, true);

          // if piece can perform valid capture move on king, king is in check (mark as threat_check):
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

void Game::print_moves(const std::string& input) {
  char piece_char = input[0];
  Field from(8 - (input[2] - '0'), input[1] - 'a');
  std::vector<std::vector<char>> printout(8, std::vector<char>(8, ' '));

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      Field to(row, col);
      bool occupied = state_[row][col] ? true : false;

      auto move = std::make_shared<Move>(piece_char, from, to, occupied);
      bool valid = try_move(move);

      if (valid && occupied) {
        printout[row][col] = 'x';
      } else if (valid && !occupied) {
        printout[row][col] = 'o';
      } else if (occupied) {
        printout[row][col] = state_[row][col]->to_char();
      }
    }
  }

  // print this bitch

  for (const auto& row : printout) {
    for (const auto& c : row) {
      std::cout << c;
    }
    std::cout << '\n';
  }
}