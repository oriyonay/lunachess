#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

typedef uint64_t U64;

#define NAME "luna"
#define VERSION "0.0.1"
#define MAX_POSITION_MOVES 256
#define MAX_GAME_MOVES 2048

// engine settings:
#define DEFAULT_TT_SIZE (1 << 20)
#define DEFAULT_QSEARCH_DEPTH 4

#define INF 2147483647

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// these are set up in a clever way so that we can add them up -
// for example, WHITE + KNIGHT = WN, BLACK + ROOK = BR, etc.
enum {WHITE, BLACK = 6};
enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
enum {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, NONE};

// enum for board squares:
enum {
  A8, B8, C8, D8, E8, F8, G8, H8,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A1, B1, C1, D1, E1, F1, G1, H1
};

// macro for files (ex. the A file bitmask can be found at FILES[A])
enum {A, B, C, D, E, F, G, H};

// macros for initial king and rook locations (used to update castle rights):
// LWR = left white rook, RBR = right black rook, etc.
enum {INIT_WK = 0x1000000000000000L, INIT_BK = 0x10L};

// utility macro for computing index of least significant bit:
#define LSB(x) (DEBRUIJN_INDEX[((x ^ (x-1)) * DEBRUIJN) >> 58])

// utility macro for popping the least significant bit:
#define POP_LSB(x) (x &= x - 1)

// utility to isolate LSB:
#define ISOLATE_LSB(x) (x & ~(x - 1))

// macros for getting information out of a move integer:
#define MOVE_TO(move) ((move >> 26) & 0x3F)
#define MOVE_FROM(move) ((move >> 20) & 0x3F)
#define MOVE_CAPTURED(move) ((move >> 16) & 0xF)
#define MOVE_IS_EP(move) (move & 0x8000)
#define MOVE_IS_PAWNFIRST(move) (move & 0x4000)
#define MOVE_IS_CASTLE(move) (move & 0x2000)
#define MOVE_IS_PROMOTION(move) (move & 0x1000)
#define MOVE_PROMOTION_PIECE(move) ((move >> 8) & 0xF)
#define MOVE_PIECEMOVED(move) (move & 0xF)
#define MOVE_PCR(move) ((move >> 4) & 0xF)

// macros for determining castle rights:
// CWK = castle white kingside, CBQ = castle black queenside, etc.
#define CAN_CWK(castle_rights) (castle_rights & 0x8)
#define CAN_CWQ(castle_rights) (castle_rights & 0x4)
#define CAN_CBK(castle_rights) (castle_rights & 0x2)
#define CAN_CBQ(castle_rights) (castle_rights & 0x1)

// macros for getting the file and rank indices from a board index:
#define FILE_NO(idx) (idx % 8)
#define RANK_NO(idx) ((idx / 8) + 1) // +1 for human indexing

// defines for making castling code more readable:
#define CWK_ROOK_MASK 0xA000000000000000L
#define CWQ_ROOK_MASK 0x900000000000000L
#define CBK_ROOK_MASK 0xA0L
#define CBQ_ROOK_MASK 0x9L

#endif
