// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <unistd.h>  // getopt

#include <cassert>  // assert
#include <cstdio>  // printf
#include <cstdlib>  // atoi, exit

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

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
"-e values are always equal to weights\n"
"-u unbound 0-1 knapsack problem: items can be reused as often as desired.\n"
"   This needs usually considerably less memory.\n"
"-V output the version number and exit\n"
"-h output this text and exit\n");
}

class Knapsack {
 public:
  std::vector<int> weight_;
  std::vector<double> value_;
  int max_weight_;
  bool equal_weight_;

  virtual ~Knapsack() {
  }

  std::vector<int>::size_type size() {
    return weight_.size();
  }

  void verify() {
    assert(size() == value_.size());
  }

  virtual void Solve() = 0;
};

class MaxSelected {
 public:
  double max_value_;
  bool selected_;
  MaxSelected() : max_value_(0.), selected_(false) {
  }
};

class KnapsackBound : public Knapsack {
 private:
  typedef std::unordered_map<int, MaxSelected> WeightValueMap;
  typedef std::vector<WeightValueMap> ItemMap;

  ItemMap hash_map_;

  // Returns max using only i...n, not exceeding max_weight.
  double SolveRecurse(int i, int max_weight) {
    if (i == size()) {
      return 0;
    }
    MaxSelected& entry = hash_map_[i][max_weight];
    if (entry.max_value_ > 0.) {
      return entry.max_value_;
    }
    double result0 = SolveRecurse(i + 1, max_weight);
    max_weight -= weight_[i];
    if (max_weight >= 0) {
      double result1 = value_[i] + SolveRecurse(i + 1, max_weight);
      if (result1 > result0) {
        entry.selected_ = true;
        return ((entry.max_value_ = result1));
      }
    }
    return ((entry.max_value_ = result0));
  }

 public:
  void Solve() override {
    verify();
    hash_map_ = ItemMap(size(), WeightValueMap());
    double result = SolveRecurse(0, max_weight_);
    std::printf("%g", result);
    const char *sep = "\n";
    int weight = max_weight_;
    for(int i = 0; i < size(); ++i) {
      if (!hash_map_[i][weight].selected_) {
        continue;
      }
      int current_weight = weight_[i];
      if (equal_weight_) {
        std::printf("%s%d", sep, current_weight);
      } else {
        std::printf("%s%d(%g)", sep, current_weight, value_[i]);
      }
      sep = " ";
      weight -= current_weight;
    }
    std::printf("\n");
  }
};

class MaxCount {
 public:
  double max_value_;
  int index_;
  int count_;
  MaxCount() : index_(-1) {
  }
};

class KnapsackUnbound : public Knapsack {
 private:
  typedef std::unordered_map<int, MaxCount> WeightValueMap;

  WeightValueMap hash_map_;

  // Returns max using only i...n, not exceeding max_weight.
  double SolveRecurse(int i, int max_weight) {
    if (i == size()) {
      return 0;
    }
    MaxCount& entry = hash_map_[max_weight];
    if (entry.index_ >= 0) {
      return entry.max_value_;
    }
    int current_weight = weight_[i];
    double current_value = value_[i];
    double current_sum_value = 0;
    double result = 0;
    int count(0);
    for (int current_count(0); max_weight >= 0; ++current_count) {
      double r = current_sum_value + SolveRecurse(i + 1, max_weight);
      if (r > result) {
        result = r;
        count = current_count;
      }
      current_sum_value += current_value;
      max_weight -= current_weight;
    }
    if (count > 0) {
      entry.index_ = i;
      entry.count_ = count;
    }
    return ((entry.max_value_ = result));
  }

 public:
  void Solve() override {
    verify();
    hash_map_ = WeightValueMap();
    double result = SolveRecurse(0, max_weight_);
    std::printf("%g", result);
    const char *sep = "\n";
    for(int weight = max_weight_; weight >= 0; ) {
      int i = hash_map_[weight].index_;
      if (i < 0) {
        break;
      }
      int count = hash_map_[weight].count_;
      int current_weight = weight_[i];
      double current_value = value_[i];
      int total_weight = count * current_weight;
      if (equal_weight_) {
        std::printf("%s%d*%d=%d", sep, count, current_weight, total_weight);
      } else {
        std::printf("%s%d*%d(%d*%g=%g)", sep, count, current_weight,
          count, current_value, count * current_value);
      }
      sep = " ";
      weight -= total_weight;
    }
    std::printf("\n");
  }
};

int ParseWeight(const char *s) {
  int weight(std::atoi(s));
  if (weight <= 0) {
    std::printf("Weight %s is not a positive integer\n", s);
    std::exit(EXIT_FAILURE);
  }
  return weight;
}

int main(int argc, char *argv[]) {
  bool equal_weight(false), bound(true);
  int opt;
  while ((opt = getopt(argc, argv, "euVh")) != -1) {
    switch (opt) {
      case 'e':
        equal_weight = true;
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
  Knapsack *sack;
  if (bound) {
    sack = new KnapsackBound;
  } else {
    sack = new KnapsackUnbound;
  }
  if (argc - optind < 2) {
    std::printf("Not enough arguments. Type knapsack -h for help.\n");
    std::exit(EXIT_FAILURE);
  }
  sack->equal_weight_ = equal_weight;
  sack->max_weight_ = ParseWeight(argv[optind++]);
  if (equal_weight) {
    for (int i = optind; i < argc; ++i) {
      int weight(ParseWeight(argv[i]));
      sack->weight_.push_back(weight);
      sack->value_.push_back(static_cast<double>(weight));
    }
  } else {
    bool parse_weight(true);
    int weight;
    for (int i = optind; i < argc; ++i) {
      if (parse_weight) {
        parse_weight = false;
        weight = ParseWeight(argv[i]);
        sack->weight_.push_back(weight);
      } else {
        parse_weight = true;
        double value(atof(argv[i]));
        sack->value_.push_back((value > 0) ? value :
          static_cast<double>(weight));
      }
    }
    if (!parse_weight) {
      sack->value_.push_back(static_cast<double>(weight));
    }
  }
  sack->Solve();
  delete sack;
  return 0;
}
