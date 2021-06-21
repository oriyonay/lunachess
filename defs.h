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

#define INF 2147483647

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// these are set up in a clever way so that we can add them up -
// for example, WHITE + KNIGHT = WN, BLACK + ROOK = BR, etc.
enum {WHITE, BLACK = 6};
enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
enum {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, NONE};

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
