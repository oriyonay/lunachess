#include "board.h"
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
  for (int piece = BP; piece <= BK; piece++) {
    for (int square = 0; square < 64; square++) {
      // we just reverse the rows of white's PSTs:
      int new_square = ((7 - (square / 8)) * 8) + (square % 8);
      PIECE_SQUARE_TABLE[piece][new_square] = -PIECE_SQUARE_TABLE[piece - BLACK][square];
    }
  }

  for (int piece = WP; piece <= BK; piece++) {
    for (int square = 0; square < 64; square++) {
      PIECE_SQUARE_TABLE[piece][square] += PIECE_TO_MATERIAL[piece];
    }
  }

  // now that we've initialized all PSTs, let's get the rook differences:
  CWK_ROOK_PST_DIFFERENCE = PIECE_SQUARE_TABLE[WR][F1] - PIECE_SQUARE_TABLE[WR][H1];
  CWQ_ROOK_PST_DIFFERENCE = PIECE_SQUARE_TABLE[WR][D1] - PIECE_SQUARE_TABLE[WR][A1];
  CBK_ROOK_PST_DIFFERENCE = PIECE_SQUARE_TABLE[BR][F8] - PIECE_SQUARE_TABLE[BR][H8];
  CBQ_ROOK_PST_DIFFERENCE = PIECE_SQUARE_TABLE[BR][D8] - PIECE_SQUARE_TABLE[BR][A8];

  // precalculate castling rook zobrist keys:
  CWK_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[WR][F1] ^ ZOBRIST_SQUARE_KEYS[WR][H1];
  CWQ_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[WR][D1] ^ ZOBRIST_SQUARE_KEYS[WR][A1];
  CBK_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[BR][F8] ^ ZOBRIST_SQUARE_KEYS[BR][H8];
  CBQ_ROOK_ZOBRIST = ZOBRIST_SQUARE_KEYS[BR][D8] ^ ZOBRIST_SQUARE_KEYS[BR][A8];

  // round transposition table size down to nearest power of 2 & initialize index mask:
  while (TT_SIZE & (TT_SIZE-1)) POP_LSB(TT_SIZE);
  TT_INDEX_MASK = TT_SIZE - 1;

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
unsigned int TT_INDEX_MASK;

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

// piece-square table values (from the sunfish engine)
int PIECE_SQUARE_TABLE[12][64] = {
  // WHITE PAWN PST
  // NOTE: FIRST AND LAST ROWS MUST BE ZERO!!!
  { 0,   0,   0,   0,   0,   0,   0,   0,
   78,  83,  86,  73, 102,  82,  85,  90,
    7,  29,  21,  44,  40,  31,  44,   7,
  -17,  16,  -2,  15,  14,   0,  15, -13,
  -26,   3,  10,   9,   6,   1,   0, -23,
  -22,   9,   5, -11, -10,  -2,   3, -19,
  -31,   8,  -7, -37, -36, -14,   3, -31,
    0,   0,   0,   0,   0,   0,   0,   0
 },
  // WHITE KNIGHT PST
  { -66, -53, -75, -75, -10, -55, -58, -70,
     -3,  -6, 100, -36,   4,  62,  -4, -14,
     10,  67,   1,  74,  73,  27,  62,  -2,
     24,  24,  45,  37,  33,  41,  25,  17,
     -1,   5,  31,  21,  22,  35,   2,   0,
    -18,  10,  13,  22,  18,  15,  11, -14,
    -23, -15,   2,   0,   2,   0, -23, -20,
    -74, -23, -26, -24, -19, -35, -22, -69
 },
  // WHITE BISHOP PST
  {-59, -78, -82, -76, -23,-107, -37, -50,
   -11,  20,  35, -42, -39,  31,   2, -22,
    -9,  39, -32,  41,  52, -10,  28, -14,
    25,  17,  20,  34,  26,  25,  15,  10,
    13,  10,  17,  23,  17,  16,   0,   7,
    14,  25,  24,  15,   8,  25,  20,  15,
    19,  20,  11,   6,   7,   6,  20,  16,
    -7,   2, -15, -12, -14, -15, -10, -10
 },
  // WHITE ROOK PST
  {35,  29,  33,   4,  37,  33,  56,  50,
   55,  29,  56,  67,  55,  62,  34,  60,
   19,  35,  28,  33,  45,  27,  25,  15,
    0,   5,  16,  13,  18,  -4,  -9,  -6,
  -28, -35, -16, -21, -13, -29, -46, -30,
  -42, -28, -42, -25, -25, -35, -26, -46,
  -53, -38, -31, -26, -29, -43, -44, -53,
  -30, -24, -18,   5,  -2, -18, -31, -32
 },
  // WHITE QUEEN PST
  {  6,   1,  -8,-104,  69,  24,  88,  26,
    14,  32,  60, -10,  20,  76,  57,  24,
    -2,  43,  32,  60,  72,  63,  43,   2,
     1, -16,  22,  17,  25,  20, -13,  -6,
   -14, -15,  -2,  -5,  -1, -10, -20, -22,
   -30,  -6, -13, -11, -16, -11, -16, -27,
   -36, -18,   0, -19, -15, -15, -21, -38,
   -39, -30, -31, -13, -31, -36, -34, -42
 },
  // WHITE KING PST
  {  4,  54,  47, -99, -99,  60,  83, -62,
   -32,  10,  55,  56,  56,  55,  10,   3,
   -62,  12, -57,  44, -67,  28,  37, -31,
   -55,  50,  11,  -4, -19,  13,   0, -49,
   -55, -43, -52, -28, -51, -47,  -8, -50,
   -47, -42, -43, -79, -64, -32, -29, -32,
    -4,   3, -14, -50, -57, -18,  13,   4,
    17,  30,  -3, -14,   6,  -1,  40,  18
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
};

char* PIECE_CHARS = "PNBRQKpnbrqk ";

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
int CWK_ROOK_PST_DIFFERENCE;
int CWQ_ROOK_PST_DIFFERENCE;
int CBK_ROOK_PST_DIFFERENCE;
int CBQ_ROOK_PST_DIFFERENCE;

// precalculation of zobrist keys of rook positions before and after castling:
U64 CWK_ROOK_ZOBRIST;
U64 CWQ_ROOK_ZOBRIST;
U64 CBK_ROOK_ZOBRIST;
U64 CBQ_ROOK_ZOBRIST;

// miscellaneous pre-calculated constants:
// char FILE_OF[64];
// char RANK_OF[64];
