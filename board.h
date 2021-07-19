#ifndef BOARD_H
#define BOARD_H

#include <algorithm>
#include <cstring>

#include "consts.h"
#include "defs.h"
#include "tt.h"

struct board {
  // main board data:
  U64 bitboard[12];
  char piece_board[64];
  int move_history[MAX_GAME_MOVES];
  int num_moves_played;
  int base_score; // material + PST score
  char turn;
  char castle_rights; // bits: 0 0 0 0 K Q k q
  U64 hash;

  // the transposition table:
  tt_entry* TT;

  // data used to generate moves:
  U64 CANT_CAPTURE;
  U64 CAN_CAPTURE;
  U64 CAN_MOVE_TO;
  U64 OCCUPIED_SQUARES;
  U64 EMPTY_SQUARES;
  U64 UNSAFE;
  U64 W;
  U64 B;

  // main board functions:
  board(char* FEN);
  int* get_moves(int& num_moves);
  bool make_move(char* move);
  bool make_move(int move);
  void undo_move();
  void print();

  // move generation utility functions:
  void add_pawn_moves(int* move_list, int& num_moves);
  void add_diag_moves(int* move_list, int& num_moves);
  void add_line_moves(int* move_list, int& num_moves);
  void add_knight_moves(int* move_list, int& num_moves);
  void add_king_moves(int* move_list, int& num_moves);

  inline void update_move_info_bitboards();
  inline void update_unsafe();
  U64 pinned_pieces();
  bool is_check();
};

// utility functions for board class:
inline static int move_int(char TO, char FROM, char CAPTURED, char EP, char PF, char CASTLE,
                    char PROM, char PROM_PIECE, char PCR, char PM);
inline static int move_int(char TO, char FROM, char CAPTURED, char PCR, char PM);
inline U64 line_moves(char s, U64 OCCUPIED);
inline U64 diag_moves(char s, U64 OCCUPIED);
inline static U64 line_moves_magic(char s, U64 OCCUPIED);
inline static U64 diag_moves_magic(char s, U64 OCCUPIED);
inline U64 reverse_bits(U64 n);

// utility functions for determining pinned pieces, etc.:
inline static U64 xray_rook(U64 occ, U64 blockers, char s);
inline static U64 xray_bishop(U64 occ, U64 blockers, char s);

#endif
