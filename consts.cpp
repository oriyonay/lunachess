#include "consts.h"

extern void init_consts() {
  // ----- initialize file bitmasks: -----
  FILES[0] = 0L;
  for (int i = 0; i < 8; i++) {
    FILES[0] |= (1L << (i*8));
  }

  for (int i = 1; i < 8; i++) {
    FILES[i] = FILES[i-1] << 1;
  }

  FILE_AB = FILES[A] | FILES[B];
  FILE_GH = FILES[G] | FILES[H];

  // ----- initialize rank bitmasks: -----
  RANKS[8] = (1L << 8) - 1;
  for (int i = 7; i >= 0; i--) {
    RANKS[i] = RANKS[i+1] << 8;
  }

  // ----- initialize the diagonal masks: -----
  int i, j, start_idx;
  int inc_amount = 9;
  for (i = 0; i < 13; i++) {
    // determine where to start marking diagonally:
    if (i < 7) start_idx = i;
    else start_idx = (8 * (i-6));

    // mark the current mask:
    ANTIDIAGONAL_MASKS[i] = 0L;
    for (j = start_idx; j < 64; j += inc_amount) {
      ANTIDIAGONAL_MASKS[i] |= (1L << j);
      if ((j % 8) == 7) break;
    }
  }

  inc_amount = 7;
  for (i = 0; i < 13; i++) {
    // determine where to start marking diagonally:
    if (i < 7) start_idx = i;
    else start_idx = (8 * (i-6)) + 7;

    // mark the current mask:
    DIAGONAL_MASKS[i] = 0L;
    for (j = start_idx; j < 64; j += inc_amount) {
      DIAGONAL_MASKS[i] |= (1L << j);
      if ((inc_amount == 9) && ((j % 8) == 7)) break;
      if ((inc_amount == 7) && ((j % 8) == 0)) break;
    }
  }

  // manually add corners and the biggest diagonal mask:
  DIAGONAL_MASKS[13] = 0x102040810204080L;
  DIAGONAL_MASKS[14] = (1L << 63);
  ANTIDIAGONAL_MASKS[13] = (1L << 7);
  ANTIDIAGONAL_MASKS[14] = (1L << 56);

  // sort the diagonal masks. this makes indexing easier later on:
  // (we sort the first half of antidiagonal masks in ascending order, then
  // in descending order)
  std::sort(DIAGONAL_MASKS, DIAGONAL_MASKS + 15);
  std::sort(ANTIDIAGONAL_MASKS, ANTIDIAGONAL_MASKS + 15);
  std::sort(ANTIDIAGONAL_MASKS + 7, ANTIDIAGONAL_MASKS + 15, std::greater<U64>());

  // calculate the knight moves bitmasks:
  for (int i = 0; i < 64; i++) {
    KNIGHT_MOVES[i] = (i > 18) ? KNIGHT_SPAN << (i - 18) : KNIGHT_SPAN >> (18 - i);
    KNIGHT_MOVES[i] &= (i % 8 < 4) ? ~FILE_GH : ~FILE_AB;
  }

  // calculate the king moves bitmasks:
  for (int i = 0; i < 64; i++) {
    KING_MOVES[i] = (i > 9) ? KING_SPAN << (i - 9) : KING_SPAN >> (9 - i);
    KING_MOVES[i] &= (i % 8 < 4) ? ~FILE_GH : ~FILE_AB;
  }

  // calculate RECT_LOOKUP:
  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      RECT_LOOKUP[i][j] = in_between(i, j);
    }
  }
}

// in_between(): given indices of two squares, returns 0 if there is no line
// between them, and 1s along the line between them (in any direction) otherwise
U64 in_between(int sq1, int sq2) {
   const U64 m1   = -1L;
   const U64 a2a7 = 0x0001010101010100L;
   const U64 b2g7 = 0x0040201008040200L;
   const U64 h1b7 = 0x0002040810204080L; /* Thanks Dustin, g2b7 did not work for c1-a3 */
   U64 btwn, line, rank, file;

   btwn  = (m1 << sq1) ^ (m1 << sq2);
   file  =   (sq2 & 7) - (sq1   & 7);
   rank  =  ((sq2 | 7) -  sq1) >> 3 ;
   line  =      (   (file  &  7) - 1) & a2a7; /* a2a7 if same file */
   line += 2 * ((   (rank  &  7) - 1) >> 58); /* b1g1 if same rank */
   line += (((rank - file) & 15) - 1) & b2g7; /* b2g7 if same diagonal */
   line += (((rank + file) & 15) - 1) & h1b7; /* h1b7 if same antidiag */
   line *= btwn & -btwn; /* mul acts like shift by smaller square */
   return line & btwn;   /* return the bits on that line in-between */
}

std::unordered_map<char, int> PIECE_INDICES = {
  {'P', 0}, {'N', 1}, {'B', 2}, {'R', 3}, {'Q', 4}, {'K', 5},
  {'p', 6}, {'n', 7}, {'b', 8}, {'r', 9}, {'q', 10},{'k', 11}
};

std::unordered_map<char, int> MATERIAL = {
  {'P', 100}, {'N', 275}, {'B', 325}, {'R', 500}, {'Q', 900}, {'K', 100000},
  {'p', -100}, {'n', -275}, {'b', -325}, {'r', -500}, {'q', -900},{'k', -100000}
};

const int PIECE_TO_MATERIAL[13] = {100, 275, 325, 500, 900, 100000,
                             -100, -275, -325, -500, -900, -100000, 0};

char* PIECE_CHARS = "PNBRQKpnbrqk ";

U64 FILES[8] = {0};
U64 RANKS[9] = {0};
U64 DIAGONAL_MASKS[15] = {0};
U64 ANTIDIAGONAL_MASKS[15] = {0};
U64 FILE_AB = 0L;
U64 FILE_GH = 0L;

const U64 DEBRUIJN = 0x03f79d71b4cb0a89L;
const char DEBRUIJN_INDEX[64] = {
  0, 47,  1, 56, 48, 27,  2, 60,
  57, 49, 41, 37, 28, 16,  3, 61,
  54, 58, 35, 52, 50, 42, 21, 44,
  38, 32, 29, 23, 17, 11,  4, 62,
  46, 55, 26, 59, 40, 36, 15, 53,
  34, 51, 20, 43, 31, 22, 10, 45,
  25, 39, 14, 33, 19, 30,  9, 24,
  13, 18,  8, 12,  7,  6,  5, 63
};

U64 KNIGHT_SPAN = 43234889994L;
U64 KING_SPAN = 460039L;

U64 KNIGHT_MOVES[64] = {0};
U64 KING_MOVES[64] = {0};

U64 RECT_LOOKUP[64][64] = {0};

U64 CWK_SAFE_SPACES = (1L << 61) | (1L << 62);
U64 CWQ_SAFE_SPACES = (1L << 58) | (1L << 59);
U64 CBK_SAFE_SPACES = (1L << 5)| (1L << 6);
U64 CBQ_SAFE_SPACES = (1L << 2) | (1L << 3);

U64 CWQ_EMPTY_SPACES = (1L << 57) | (1L << 58) | (1L << 59);
U64 CBQ_EMPTY_SPACES = (1L << 1) | (1L << 2) | (1L << 3);
