#include <stdio.h>
#include <vector>

#include "board.h"

#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

int perft(board* b, int depth);

struct perft_test {
  char* test_name;
  char* FEN;
  std::vector<int> results;

  perft_test(char* test_name, char* FEN, std::vector<int> results) :
    test_name(test_name), FEN(FEN), results(results) {}

  bool test() {
    board b(FEN);
    printf("starting test: %s\n", test_name);
    for (int i = 1; i < results.size(); i++) {
      if (perft(&b, i) != results[i]) {
        printf("%s %sFAILED%s at depth %d\n", test_name, RED, RESET, i);
        return false;
      }
    }
    printf("%s %sPASSED%s.\n", test_name, GREEN, RESET);
    return true;
  }
};

int main() {
  // initialize engine constants!
  init_consts();

  // create test objects:
  perft_test initial_position(
    "initial position",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",
    std::vector<int> {1, 20, 400, 8902, 197281, 4865609, 119060324}
  );

  perft_test pt2 (
    "perft test 2",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
    std::vector<int> {1, 48, 2039, 97862, 4085603, 193690690}
  );

  perft_test pt3 (
    "perft test 3",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
    std::vector<int> {1, 14, 191, 2812, 43238, 674624, 11030083, 178633661}
  );

  perft_test pt4 (
    "perft test 4",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    std::vector<int> {1, 6, 264, 9467, 422333, 15833292}
  );

  perft_test pt5 (
    "perft test 5",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",
    std::vector<int> {1, 6, 264, 9467, 422333, 15833292}
  );

  perft_test pt6 (
    "perft test 6",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
    std::vector<int> {1, 44, 1486, 62379, 2103487, 89941194}
  );

  perft_test pt7 (
    "perft test 7",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
    std::vector<int> {1, 46, 2079, 89890, 3894594, 164075551}
  );

  // run all tests:
  bool all_tests_passed = true;
  all_tests_passed &= initial_position.test();
  all_tests_passed &= pt2.test();
  all_tests_passed &= pt3.test();
  all_tests_passed &= pt4.test();
  all_tests_passed &= pt5.test();
  all_tests_passed &= pt6.test();
  all_tests_passed &= pt7.test();

  if (all_tests_passed) printf("%sALL TESTS PASSED%s.\n", GREEN, RESET);
  else printf("%sNOT ALL TESTS PASSED%s.\n", YELLOW, RESET);
}

// perft(): the perft test function
int perft(board* b, int depth) {
  int num_moves;
  int* moves = b->get_moves(num_moves);
  if (depth == 1) {
    int n_copy = num_moves;
    for (int i = 0; i < n_copy; i++) {
      if (!b->make_move(moves[i])) num_moves--;
      b->undo_move();
    }
    return num_moves;
  }

  int sum = 0;
  for (int i = 0; i < num_moves; i++) {
    if (b->make_move(moves[i])) {
      sum += perft(b, depth - 1);
    }
    b->undo_move();
  }

  return sum;
}
