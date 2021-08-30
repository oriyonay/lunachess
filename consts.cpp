#include "consts.h"

extern void init_consts() {
  // ----- initialize random zobrist keys: -----
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 64; j++) {
      ZOBRIST_SQUARE_KEYS[i][j] = generator();
    }
  }

  for (int i = 0; i < 16; i++) {
    ZOBRIST_CASTLE_RIGHTS_KEYS[i] = generator();
  }

  for (int i = 0; i < 8; i++) {
    ZOBRIST_EP_KEYS[i] = generator();
  }

  ZOBRIST_TURN_KEY = generator();

  // ----- initialize black's PSTs + add material values -----
  for (int phase = OPENING_PHASE; phase <= ENDGAME_PHASE; phase++) {
    for (int piece = BP; piece <= BK; piece++) {
      for (int square = 0; square < 64; square++) {
        // we just reverse the rows of white's PSTs:
        int new_square = ((7 - (square / 8)) * 8) + (square % 8);
        PIECE_SQUARE_TABLE[phase][piece][new_square] = -PIECE_SQUARE_TABLE[phase][piece - BLACK][square];
      }
    }

    for (int piece = WP; piece <= BK; piece++) {
      for (int square = 0; square < 64; square++) {
        PIECE_SQUARE_TABLE[phase][piece][square] += PIECE_TO_MATERIAL[phase][piece];
      }
    }
  }

  // now that we've initialized all PSTs, let's get the rook differences:
  CWK_ROOK_PST_DIFFERENCE_OPENING = PIECE_SQUARE_TABLE[OPENING_PHASE][WR][F1] - PIECE_SQUARE_TABLE[OPENING_PHASE][WR][H1];
  CWQ_ROOK_PST_DIFFERENCE_OPENING = PIECE_SQUARE_TABLE[OPENING_PHASE][WR][D1] - PIECE_SQUARE_TABLE[OPENING_PHASE][WR][A1];
  CBK_ROOK_PST_DIFFERENCE_OPENING = PIECE_SQUARE_TABLE[OPENING_PHASE][BR][F8] - PIECE_SQUARE_TABLE[OPENING_PHASE][BR][H8];
  CBQ_ROOK_PST_DIFFERENCE_OPENING = PIECE_SQUARE_TABLE[OPENING_PHASE][BR][D8] - PIECE_SQUARE_TABLE[OPENING_PHASE][BR][A8];

  CWK_ROOK_PST_DIFFERENCE_ENDGAME = PIECE_SQUARE_TABLE[ENDGAME_PHASE][WR][F1] - PIECE_SQUARE_TABLE[ENDGAME_PHASE][WR][H1];
  CWQ_ROOK_PST_DIFFERENCE_ENDGAME = PIECE_SQUARE_TABLE[ENDGAME_PHASE][WR][D1] - PIECE_SQUARE_TABLE[ENDGAME_PHASE][WR][A1];
  CBK_ROOK_PST_DIFFERENCE_ENDGAME = PIECE_SQUARE_TABLE[ENDGAME_PHASE][BR][F8] - PIECE_SQUARE_TABLE[ENDGAME_PHASE][BR][H8];
  CBQ_ROOK_PST_DIFFERENCE_ENDGAME = PIECE_SQUARE_TABLE[ENDGAME_PHASE][BR][D8] - PIECE_SQUARE_TABLE[ENDGAME_PHASE][BR][A8];

  // precalculate castling rook zobrist keys:
  CWK_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[WR][F1] ^ ZOBRIST_SQUARE_KEYS[WR][H1];
  CWQ_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[WR][D1] ^ ZOBRIST_SQUARE_KEYS[WR][A1];
  CBK_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[BR][F8] ^ ZOBRIST_SQUARE_KEYS[BR][H8];
  CBQ_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[BR][D8] ^ ZOBRIST_SQUARE_KEYS[BR][A8];

  // set up transposition table size and calculate number of entries:
  // (16 = sizeof(tt_entry))
  NUM_TT_ENTRIES = DEFAULT_TT_SIZE / 16;

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

  for (int sq = 0; sq < 64; sq++) {
    SQUARE_FILES[sq] = FILES[FILE_NO(sq)];
  }

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

  // calculate isolated and passed pawn bitmasks:
  for (int sq = 0; sq < 64; sq++) {
    int file = FILE_NO(sq);
    int rank = RANK_NO(sq);

    // initialize isolated mask:
    U64 mask = 0L;
    if (file > 0) mask |= FILES[file-1];
    if (file < 7) mask |= FILES[file+1];
    ISOLATED_MASKS[sq] = mask;

    // initialize passed pawn masks:
    U64 white_mask = mask | FILES[file];
    U64 black_mask = mask | FILES[file];
    for (int i = 0; i < (10-rank); i++) {
      white_mask &= ~RANKS[i];
    }
    for (int i = 0; i < rank; i++) {
      black_mask &= ~RANKS[8-i];
    }

    WHITE_PASSED_PAWN_MASKS[sq] = white_mask;
    BLACK_PASSED_PAWN_MASKS[sq] = black_mask;
    }

  /* ---------- initialize magic bitboard-related tables: ---------- */
  // initialize ROOK_MASKS and BISHOP_MASKS:
  for (int i = 0; i < 64; i++) {
    ROOK_MASKS[i] = rmask(i);
    BISHOP_MASKS[i] = bmask(i);
  }

  // initialize the rook magic tables:
  U64 occ;
  int hash_index;
  for (int sq = 0; sq < 64; sq++) {
    for (int idx = 0; idx < (1 << ROOK_INDEX_BITS[sq]); idx++) {
      occ = get_blockers_from_index(idx, ROOK_MASKS[sq]);
      hash_index = (occ * ROOK_MAGICS[sq]) >> (64 - ROOK_INDEX_BITS[sq]);
      ROOK_TABLE[sq][hash_index] = line_moves(sq, occ);
    }
  }

  // initialize the bishop magic tables:
  for (int sq = 0; sq < 64; sq++) {
    for (int idx = 0; idx < (1 << BISHOP_INDEX_BITS[sq]); idx++) {
      occ = get_blockers_from_index(idx, BISHOP_MASKS[sq]);
      hash_index = (occ * BISHOP_MAGICS[sq]) >> (64 - BISHOP_INDEX_BITS[sq]);
      BISHOP_TABLE[sq][hash_index] = diag_moves(sq, occ);
    }
  }
}

/* ---------- functions for calculating constants ---------- */

// in_between(): given indices of two squares, returns 0 if there is no line
// between them, and 1s along the line between them (in any direction) otherwise
// (from chessprogramming.org):
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

// rmask(): calculates rook mask for a given square index (from chessprogramming.org)
U64 rmask(int sq) {
  U64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk+1; r <= 6; r++) result |= (1ULL << (fl + r*8));
  for (r = rk-1; r >= 1; r--) result |= (1ULL << (fl + r*8));
  for (f = fl+1; f <= 6; f++) result |= (1ULL << (f + rk*8));
  for (f = fl-1; f >= 1; f--) result |= (1ULL << (f + rk*8));
  return result;
}

// bmask(): calculates bishop mask for a given square index (from chessprogramming.org)
U64 bmask(int sq) {
  U64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
  for (r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
  for (r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
  for (r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
  return result;
}

// get_blockers_from_index(): given a magic table hash index, calculate the
// bitboard of blocking pieces:
U64 get_blockers_from_index(int index, U64 mask) {
  U64 blockers = 0L;
  int bit_idx;
  int i = 0;
  while (mask) {
    bit_idx = LSB(mask);
    if (index & (1 << i)) {
      blockers |= (1L << bit_idx);
    }
    POP_LSB(mask);
    i++;
  }
  return blockers;
}

// a 64-bit pseudo-random number generator (mersenne twister engine):
std::random_device rd;
std::mt19937_64 generator(rd());

/* ---------- end of functions for calculating constants ---------- */

unsigned int TT_SIZE = DEFAULT_TT_SIZE;
unsigned int NUM_TT_ENTRIES;
unsigned int TT_INDEX_MASK;

std::unordered_map<char, int> PIECE_INDICES = {
  {'P', 0}, {'N', 1}, {'B', 2}, {'R', 3}, {'Q', 4}, {'K', 5},
  {'p', 6}, {'n', 7}, {'b', 8}, {'r', 9}, {'q', 10},{'k', 11}
};

const int GAME_PHASE_MATERIAL_SCORE[13] = {0,  337,  365,  477,  1025,  0,
                                           0,  337,  365,  477,  1025,  0, 0};

const int PIECE_TO_MATERIAL[2][13] = {{82,  337,  365,  477,  1025,  12000,
                                      -82, -337, -365, -477, -1025, -12000, 0},
                                      {94,  281,  297,  512,   936,  12000,
                                      -94, -281, -297, -512,  -936, -12000, 0}
                                     };

// PeSTO piece-square tables (indexed [game phase][piece][square])
int PIECE_SQUARE_TABLE[2][12][64] = {{
  // WHITE PAWN OPENING PST
  // NOTE: FIRST AND LAST ROWS MUST BE ZERO!!!
  {  0,   0,   0,   0,   0,   0,  0,   0,
    98, 134,  61,  95,  68, 126, 34, -11,
    -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
     0,   0,   0,   0,   0,   0,  0,   0
 },
  // WHITE KNIGHT PST
  { -167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
    -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23
 },
  // WHITE BISHOP PST
  {-29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
    -4,   5,  19,  50,  37,  37,   7,  -2,
    -6,  13,  13,  26,  34,  12,  10,   4,
    0,  15,  15,  15,  14,  27,  18,  10,
    4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21
 },
  // WHITE ROOK PST
  {32,  42,  32,  51, 63,  9,  31,  43,
  27,  32,  58,  62, 80, 67,  26,  44,
  -5,  19,  26,  36, 17, 45,  61,  16,
  -24, -11,   7,  26, 24, 35,  -8, -20,
  -36, -26, -12,  -1,  9, -7,   6, -23,
  -45, -25, -16, -17,  3,  0,  -5, -33,
  -44, -16, -20,  -9, -1, 11,  -6, -71,
  -19, -13,   1,  17, 16,  7, -37, -26
 },
  // WHITE QUEEN PST
  { -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
    -1, -18,  -9,  10, -15, -25, -31, -50
 },
  // WHITE KING PST
  { -65,  23,  16, -15, -56, -34,   2,  13,
    29,  -1, -20,  -7,  -8,  -4, -38, -29,
    -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14
 },
  // BLACK PSTs ARE INITIALIZED IN INIT_CONSTS()
  // BLACK PAWN PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK KNIGHT PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK BISHOP PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK ROOK PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK QUEEN PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK KING PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 }
},
// ENDGAME PSTs
{
    // WHITE PAWN ENDGAME PST
  { 0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100,  85,  67,  56,  53,  82,  84,
    32,  24,  13,   5,  -2,   4,  17,  17,
    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
    4,   7,  -6,   1,   0,  -5,  -1,  -8,
    13,   8,   8,  10,  13,   0,   2,  -7,
    0,   0,   0,   0,   0,   0,   0,   0
 },
  // WHITE KNIGHT ENDGAME PST
  {  -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64
 },
  // WHITE BISHOP ENDGAME PST
  { -14, -21, -11,  -8, -7,  -9, -17, -24,
    -8,  -4,   7, -12, -3, -13,  -4, -14,
    2,  -8,   0,  -1, -2,   6,   0,   4,
    -3,   9,  12,   9, 14,  10,   3,   2,
    -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17
 },
  // WHITE ROOK ENDGAME PST
  {13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
    7,  7,  7,  5,  4,  -3,  -5,  -3,
    4,  3, 13,  1,  2,   1,  -1,   2,
    3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20
 },
  // WHITE QUEEN ENDGAME PST
  { -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
    3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41
 },
  // WHITE KING ENDGAME PST
  { -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
    10,  17,  23,  15,  20,  45,  44,  13,
    -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
 },
  // BLACK ENDGAME PSTs ARE INITIALIZED IN INIT_CONSTS()
  // BLACK PAWN ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK KNIGHT ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK BISHOP ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK ROOK ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK QUEEN ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 },
  // BLACK KING ENDGAME PST
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 }
}};

char* PIECE_CHARS = "PNBRQKpnbrqk ";

const int OPENING_PHASE_SCORE = 6192;
const int ENDGAME_PHASE_SCORE = 518;

U64 FILES[8];
U64 RANKS[9];
U64 DIAGONAL_MASKS[15];
U64 ANTIDIAGONAL_MASKS[15];
U64 FILE_AB;
U64 FILE_GH;

// MAGIC BITBOARD tables
U64 ROOK_MASKS[64];
U64 BISHOP_MASKS[64];

const U64 ROOK_MAGICS[64] = {
    0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
    0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
    0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
    0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
    0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
    0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
    0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
    0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
    0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
    0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
    0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
    0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
    0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};

const U64 BISHOP_MAGICS[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
    0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
    0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
    0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
    0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
    0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
    0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
    0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
    0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
    0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
    0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
    0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
    0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
};

// number of index bits for each square's hash table:
const int ROOK_INDEX_BITS[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int BISHOP_INDEX_BITS[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// and finally, the actual magic tables for rook and bishop-like pieces:
U64 ROOK_TABLE[64][4096];
U64 BISHOP_TABLE[64][512];

// random numbers for zobrist hashing:
U64 ZOBRIST_SQUARE_KEYS[12][64];
U64 ZOBRIST_CASTLE_RIGHTS_KEYS[16];
U64 ZOBRIST_EP_KEYS[8];
U64 ZOBRIST_TURN_KEY;

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

U64 KNIGHT_MOVES[64];
U64 KING_MOVES[64];

U64 RECT_LOOKUP[64][64];

U64 CWK_SAFE_SPACES = (1L << 61) | (1L << 62);
U64 CWQ_SAFE_SPACES = (1L << 58) | (1L << 59);
U64 CBK_SAFE_SPACES = (1L << 5)| (1L << 6);
U64 CBQ_SAFE_SPACES = (1L << 2) | (1L << 3);

U64 CWQ_EMPTY_SPACES = (1L << 57) | (1L << 58) | (1L << 59);
U64 CBQ_EMPTY_SPACES = (1L << 1) | (1L << 2) | (1L << 3);

// constants for differences in PST values for rooks after castling:
int CWK_ROOK_PST_DIFFERENCE_OPENING;
int CWQ_ROOK_PST_DIFFERENCE_OPENING;
int CBK_ROOK_PST_DIFFERENCE_OPENING;
int CBQ_ROOK_PST_DIFFERENCE_OPENING;

int CWK_ROOK_PST_DIFFERENCE_ENDGAME;
int CWQ_ROOK_PST_DIFFERENCE_ENDGAME;
int CBK_ROOK_PST_DIFFERENCE_ENDGAME;
int CBQ_ROOK_PST_DIFFERENCE_ENDGAME;

// precalculation of zobrist keys of rook positions before and after castling:
U64 CWK_ROOK_ZOBRIST;
U64 CWQ_ROOK_ZOBRIST;
U64 CBK_ROOK_ZOBRIST;
U64 CBQ_ROOK_ZOBRIST;

/* -------------------- CONSTANTS FOR ENGINE EVALUATION -------------------- */
// (from BBChess engine)
// indexed [attacker][victim]
int MVV_LVA_SCORE[12][13] = {
 	0  , 0  , 0  , 0  , 0  , 0  ,  105, 205, 305, 405, 505, 605, 0,
	0  , 0  , 0  , 0  , 0  , 0  ,  104, 204, 304, 404, 504, 604, 0,
	0  , 0  , 0  , 0  , 0  , 0  ,  103, 203, 303, 403, 503, 603, 0,
	0  , 0  , 0  , 0  , 0  , 0  ,  102, 202, 302, 402, 502, 602, 0,
	0  , 0  , 0  , 0  , 0  , 0  ,  101, 201, 301, 401, 501, 601, 0,
	0  , 0  , 0  , 0  , 0  , 0  ,  100, 200, 300, 400, 500, 600, 0,

	105, 205, 305, 405, 505, 605,  0  , 0  , 0  , 0  , 0  , 0  , 0,
	104, 204, 304, 404, 504, 604,  0  , 0  , 0  , 0  , 0  , 0  , 0,
	103, 203, 303, 403, 503, 603,  0  , 0  , 0  , 0  , 0  , 0  , 0,
	102, 202, 302, 402, 502, 602,  0  , 0  , 0  , 0  , 0  , 0  , 0,
	101, 201, 301, 401, 501, 601,  0  , 0  , 0  , 0  , 0  , 0  , 0,
	100, 200, 300, 400, 500, 600,  0  , 0  , 0  , 0  , 0  , 0  , 0
};

U64 ISOLATED_MASKS[64];
U64 WHITE_PASSED_PAWN_MASKS[64];
U64 BLACK_PASSED_PAWN_MASKS[64];

const int PASSED_PAWN_BONUS[9] = {0, 10, 30, 50, 75, 100, 150, 200};

const int SEE_PIECE_VALUES[13] = {100, 300, 300, 500, 900, 0,
                                  100, 300, 300, 500, 900, 0, 0};

// miscellaneous pre-calculated constants:
U64 SQUARE_FILES[64];
// U64 SQUARE_RANKS[64];
// char FILE_OF[64];
// char RANK_OF[64];
