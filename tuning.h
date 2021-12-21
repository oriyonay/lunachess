#ifndef TUNING_H
#define TUNING_H

#include <cassert>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include "eval.h"
#include "eval_params.h"
#include "globals.h"

struct position_data {
  char fen[128];
  double result;
};

// the constant value of K (for texel's sigmoid):
double K = 1;

// vector of all tuneable engine parameters, for cleaner code:
std::vector<int*> params = {
  &BISHOP_PAIR_BONUS,
  &DOUBLED_PAWN_PENALTY,
  &FULLY_OPEN_FILE_BONUS,
  &ISOLATED_PAWN_PENALTY,
  &KING_SHIELD_BONUS,
  &SEMI_OPEN_FILE_BONUS
};

// dependency list: in order to keep the list of parameters as small as possible,
// as well as to keep evaluation mirrored along white and black, every parameter
// has at most 1 "dependency" (i.e., some other parameter which is either a copy
// of this value (multiplier value of 1) or is the negative of this value
// (multiplier value of -1)). the dependency list is therefore indexed as follows:
// dependencies[param] = {location of dependency, multiplier value (+/-1)}
std::vector<std::pair<int*, int>> dependencies;

int main(int argc, char** argv);
void tune(char* POSITIONS_FILE, int NUM_POSITIONS_TO_EXTRACT = 64000);
double MSE(std::vector<int>& params, std::vector<position_data*>& positions);
void copy_params(std::vector<int>& destination);
void load_params(std::vector<int>& source);
void initialize_params_and_dependencies();

#endif
