# lunachess
luna: a UCI-compatible chess engine in c++

TODO:
- add detailed comments!
- board constructor: parse the last of the FEN string
- make as many functions as possible INLINE and STATIC!
- minor: change 2*x in line_moves and diag_moves to << 1
- future: there's no need to update unsafe every time if we store a
  white_attack and black_attack bitboards
- future: MOVE GENERATION - calculating legal moves takes redundant work!
  checking whether move is legal in make_move currently requires updating the
  UNSAFE bitboard twice if we're generating more moves from that position on.
- possible: make a table for all (1L << x) instead of calculating them
- replace all magic numbers with defs!
- potential redundancy in undo en passant routine?
