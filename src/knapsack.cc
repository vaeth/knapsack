// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <unistd.h>  // getopt

#include <cstdio>  // printf
#include <cstdlib>  // atoi, exit

#include "src/knapsack.h"
#include "src/knapsack_output.h"

static void Help() {
  std::printf(
"Usage: knapsack [option] max-weight weight1 value1 weight2 value2 ...\n"
"or     knapsack [option] max-weight weight1 weight2 weight3 ...\n"
"(the latter if option -e is used)\n"
"\n"
"The output is an optimal solution of the 0-1 knapsack problem\n"
"\n"
"All numbers must be positive, weights must be integers.\n"
"If a value is not positive, it is considered to be equal as its weight.\n"
"\n"
"Options:\n"
"-e values are the same as the weights\n"
"-u unbound 0-1 knapsack problem: items can be reused as often as desired.\n"
"   This needs usually considerably less memory.\n"
"-V output the version number and exit\n"
"-h output this text and exit\n");
}

int ParseWeight(const char *s) {
  int weight(std::atoi(s));
  if (weight <= 0) {
    std::printf("Weight %s is not a positive integer\n", s);
    std::exit(EXIT_FAILURE);
  }
  return weight;
}

int main(int argc, char *argv[]) {
  bool value_is_weight(false), bound(true);
  int opt;
  while ((opt = getopt(argc, argv, "euVh")) != -1) {
    switch (opt) {
      case 'e':
        value_is_weight = true;
        break;
      case 'u':
        bound = false;
        break;
      case 'V':
        std::printf("knapsack 1.0\n");
        std::exit(EXIT_SUCCESS);
        break;
      case 'h':
        Help();
        std::exit(EXIT_SUCCESS);
        break;
      default:
        std::exit(EXIT_FAILURE);
        break;
    }
  }
  KnapsackBase<int> *knapsack;
  if (bound) {
    if (value_is_weight) {
      knapsack = new KnapsackBoundOutputInt;
    } else {
      knapsack = new KnapsackBoundOutputDouble;
    }
  } else {
    if (value_is_weight) {
      knapsack = new KnapsackUnboundOutputInt;
    } else {
      knapsack = new KnapsackUnboundOutputDouble;
    }
  }
  if (argc - optind < 2) {
    std::printf("Not enough arguments. Type knapsack -h for help.\n");
    std::exit(EXIT_FAILURE);
  }
  knapsack->max_weight_ = ParseWeight(argv[optind++]);
  if (value_is_weight) {
    for (int i = optind; i < argc; ++i) {
      int weight(ParseWeight(argv[i]));
      knapsack->weight_.push_back(weight);
    }
  } else {
    bool parse_weight(true);
    int weight;
    for (int i = optind; i < argc; ++i) {
      if (parse_weight) {
        parse_weight = false;
        weight = ParseWeight(argv[i]);
        knapsack->weight_.push_back(weight);
      } else {
        parse_weight = true;
        double value(atof(argv[i]));
        static_cast<Knapsack<int, double> *>(knapsack)->
          value_.push_back((value > 0) ? value : static_cast<double>(weight));
      }
    }
    if (!parse_weight) {
      static_cast<Knapsack<int, double> *>(knapsack)->
        value_.push_back(static_cast<double>(weight));
    }
  }
  knapsack->set_value_is_weight(value_is_weight);
  knapsack->Solve();
  delete knapsack;
  return 0;
}
