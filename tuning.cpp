#include "tuning.h"

int main(int argc, char** argv) {
  // tuner settings:
  char* POSITIONS_FILE = "tuning/quiet-labeled.epd";
  int NUM_POSITIONS_TO_EXTRACT = 700000;

  // initialize consts and globals:
  init_consts();
  init_globals();

  // initialize the dependency list and the rest of the parameter list:
  initialize_params_and_dependencies();

  // start the tuner!
  printf("starting luna tuner...\n");
  tune(POSITIONS_FILE, NUM_POSITIONS_TO_EXTRACT);

  return 0;
}

void tune(char* POSITIONS_FILE, int NUM_POSITIONS_TO_EXTRACT) {
  // load the positions file:
  std::ifstream ifs(POSITIONS_FILE);

  // ensure the file is valid:
  if (ifs.fail()) {
    printf("error: file %s not found.\n", POSITIONS_FILE);
    return;
  }

  // parse the file for positions and their respective final result:
  std::vector<position_data*> positions;
  std::string result;
  for (int i = 0; i < NUM_POSITIONS_TO_EXTRACT; i++) {
    position_data* p = new position_data();
    ifs.getline(p->fen, 128, '"');
    std::getline(ifs, result);

    // parse game result:
    if (result.find("1-0") != std::string::npos) p->result = 1.0;
    else if (result.find("1/2") != std::string::npos) p->result = 0.5;
    else if (result.find("0-1") != std::string::npos) p->result = 0.0;
    else {
      i--;
      continue;
    }

    positions.push_back(p);
  }

  printf("successfully loaded %d positions\n", positions.size());

  // temporarily store the current param values in order to calculate the initial MSE:
  std::vector<int> best_param_values;
  copy_params(best_param_values);

  // calculate optimal K:
  /* std::ofstream ofs("K.txt");
  for (K = 0.001; K <= 3; K += 0.001) {
    ofs << K << " " << MSE(best_param_values, positions) << "\n";
    printf("K = %.3f\n", K);
  }
  ofs.close();
  return; */

  // tune the parameters!
  double best_MSE = MSE(best_param_values, positions);
  int num_params = params.size();
  int num_epochs = 1;
  bool improved = true;
  while (improved) {
    printf("starting epoch %d\n", num_epochs);

    improved = false;
    for (int param = 0; param < num_params; param++) {
      // make a deep copy of the current best parameters:
      std::vector<int> new_params;
      copy_params(new_params);

      // guess the direction that further minimizes the MSE:
      new_params[param]++;
      double new_MSE = MSE(new_params, positions);
      if (new_MSE < best_MSE) {
        best_MSE = new_MSE;
        load_params(new_params);
        improved = true;
      }
      else {
        new_params[param] -= 2;
        new_MSE = MSE(new_params, positions);
        if (new_MSE < best_MSE) {
          best_MSE = new_MSE;
          load_params(new_params);
          improved = true;
        }
      }
    }

    // epoch over, output MSE info and save snapshot of weights to file:
    std::string output_filename = "tuning_" + std::to_string(num_epochs) + ".txt";
    save_params(output_filename.c_str());
    printf("finished epoch %d. current best MSE: %.4f\n", num_epochs, best_MSE);
    num_epochs++;
  }
}

// calculate the MSE of the sigmoid of the current engine's evaluation and the game's final result:
double MSE(std::vector<int>& params, std::vector<position_data*>& positions) {
  double total_squared_error = 0;

  // load the params vector to the actual parameters:
  load_params(params);

  // calculate MSE:
  int eval;
  double sigmoid;
  double squared_error;
  for (int p = 0; p < positions.size(); p++) {
    // load the position to the global board object:
    b = board(positions[p]->fen);

    // statically evaluate the position using our current parameters:
    eval = evaluate() * (b.turn == WHITE ? 1 : -1);

    // calculate the sigmoid of this evaluation:
    sigmoid = 1.0 / (1.0 + pow(10, -K * eval / 400.0));

    // calculate the squared error:
    squared_error = pow(positions[p]->result - sigmoid, 2);

    // add the squared error to the total squared error:
    total_squared_error += squared_error;
  }

  // calculate and return the MSE:
  return total_squared_error / positions.size();
}

void copy_params(std::vector<int>& destination) {
  for (int i = 0; i < params.size(); i++) {
    destination.push_back(*params[i]);
  }
}

void load_params(std::vector<int>& source) {
  for (int i = 0; i < source.size(); i++) {
    *params[i] = source[i];

    // if this parameter has a dependency, update it too:
    if (dependencies[i].first) {
      *dependencies[i].first = source[i] * dependencies[i].second;
    }
  }
}

void initialize_params_and_dependencies() {
  // first set of params (hard-coded) don't have any dependencies:
  for (int i = 0; i < params.size(); i++) {
    dependencies.push_back(std::pair<int*, int>(nullptr, 0));
  }

  // push game phase material score values:
  for (int piece = WN; piece <= WQ; piece++) {
    int* location = &GAME_PHASE_MATERIAL_SCORE[piece];
    int* dependency = &GAME_PHASE_MATERIAL_SCORE[piece + 6];

    params.push_back(location);
    dependencies.push_back(std::pair<int*, int>(dependency, 1));
  }

  for (int piece = WP; piece <= BK; piece++) {
    int* location = &PIECE_TO_MATERIAL[piece/6][piece%6];
    int* dependency = &PIECE_TO_MATERIAL[piece/6][(piece%6) + 6];

    params.push_back(location);
    dependencies.push_back(std::pair<int*, int>(dependency, -1));
  }

  for (int phase = 0; phase < 2; phase++) {
    for (int piece = WP; piece <= WK; piece++) {
      for (int square = 0; square < 64; square++) {
        int* location = &PIECE_SQUARE_TABLE[phase][piece][square];

        // square conversion is straight copy-paste from consts.cpp
        int new_square = ((7 - (square / 8)) * 8) + (square % 8);
        int* dependency = &PIECE_SQUARE_TABLE[phase][piece + BLACK][new_square];

        params.push_back(location);
        dependencies.push_back(std::pair<int*, int>(dependency, -1));
      }
    }
  }

  for (int rank = 1; rank <= 8; rank++) {
    int* location = &PASSED_PAWN_BONUS[rank];
    params.push_back(location);
    dependencies.push_back(std::pair<int*, int>(nullptr, 0));
  }

  for (int piece = WP; piece <= BK; piece++) {
    int* location = &SEE_PIECE_VALUES[piece];
    params.push_back(location);
    dependencies.push_back(std::pair<int*, int>(nullptr, 0));
  }

  // sanity check: quick test to ensure all values and dependencies are correct:
  assert(params.size() == dependencies.size());
  for (int p = 0; p < params.size(); p++) {
    if (dependencies[p].first) {
      assert(*params[p] == (*dependencies[p].first) * dependencies[p].second);
    }
  }
}
