// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <osformat.h>

#include <unistd.h>  // getopt

#include <cctype>  // isdigit
#include <cstdlib>  // exit

#include <iostream>  // istringstream

#include "src/knapsack.h"
#include "src/knapsack_output.h"

// Use largest unsigned integers/floats available:
typedef unsigned long long Integer;  // NOLINT(runtime/int)
typedef long double Float;

static void Help() {
  osformat::Say(
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
"-h output this text and exit");
}

Integer ParseWeight(const char *s) {
  const char *ori(s);
  Integer weight = 0;
  if (*s == '+') {
    ++s;
  }
  for (char c(*s); std::isdigit(c); c = *(++s)) {
    Integer new_weight = (weight * 10) + (c - '0');
    if (new_weight <= weight) {
      osformat::SayError("Numer overflow in %s") % ori;
      std::exit(EXIT_FAILURE);
    }
    weight = new_weight;
  }
  if (weight == 0) {
    osformat::SayError("Weight %s is not a positive integer") % ori;
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
        osformat::Say("knapsack 3.0");
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
  KnapsackBase<Integer> *knapsack;
  if (bound) {
    if (value_is_weight) {
      knapsack = new KnapsackBoundOutput<Integer, Integer>;
    } else {
      knapsack = new KnapsackBoundOutput<Integer, Float>;
    }
  } else {
    if (value_is_weight) {
      knapsack = new KnapsackUnboundOutput<Integer, Integer, Integer>;
    } else {
      knapsack = new KnapsackUnboundOutput<Integer, Float, Integer>;
    }
  }
  if (argc - optind < 2) {
    osformat::SayError("Not enough arguments. Type knapsack -h for help");
    std::exit(EXIT_FAILURE);
  }
  knapsack->max_weight_ = ParseWeight(argv[optind++]);
  if (value_is_weight) {
    for (int i = optind; i < argc; ++i) {
      Integer weight(ParseWeight(argv[i]));
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
        std::istringstream is(argv[i]);
        Float value;
        is >> value;
        static_cast<Knapsack<Integer, Float> *>(knapsack)->
          value_.push_back((value > 0) ? value : static_cast<Float>(weight));
      }
    }
    if (!parse_weight) {
      static_cast<Knapsack<Integer, Float> *>(knapsack)->
        value_.push_back(static_cast<Float>(weight));
    }
  }
  knapsack->set_value_is_weight(value_is_weight);
  knapsack->Solve();
  delete knapsack;
  return 0;
}
