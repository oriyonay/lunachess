# lunachess
luna: a UCI-compatible chess engine in c++

TODO:
- add detailed comments!
- MAKE ORIGINAL PSTs
  - IMPROVE PSTs
- board constructor: parse the last of the FEN string
- add FEN exporter
- possible: make a table for all (1L << x) instead of calculating them
- replace all board indices (i.e., 54) with defs (i.e., g2)!
- potential redundancy in undo en passant routine?
- adding up material score AND PST score separately is inefficient. we could
  really just add material score to PST
