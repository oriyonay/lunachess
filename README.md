# lunachess
luna: a UCI-compatible chess engine in c++

TODO:
- add detailed comments!
- board constructor: parse the last of the FEN string
- add FEN exporter
- minor: change 2*x in line_moves and diag_moves to << 1
- future: there's no need to update unsafe every time if we store a
  white_attack and black_attack bitboards
- possible: make a table for all (1L << x) instead of calculating them
- replace all board indices (i.e., 54) with defs (i.e., g2)!
- potential redundancy in undo en passant routine?
