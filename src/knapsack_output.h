// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_KNAPSACK_OUTPUT_H_
#define SRC_KNAPSACK_OUTPUT_H_ 1

#include <cstdio>  // printf

#include "src/knapsack.h"

// This contains several instances of Knapsack with output to stdout.

template <class Weight, class Value> class KnapsackBoundOutput
    : public KnapsackBound<Weight, Value> {
 protected:
  const char *sep;

 public:
  typedef KnapsackBound<Weight, Value> super;

  void Solve() {
    sep = "\n";
    super::Solve();
    printf("\n");
  }
};

template <class Weight, class Value, class Count> class KnapsackUnboundOutput
    : public KnapsackUnbound<Weight, Value, Count> {
 protected:
  const char *sep;

 public:
  typedef KnapsackUnbound<Weight, Value, Count> super;

  void Solve() {
    sep = "\n";
    super::Solve();
    printf("\n");
  }
};

class KnapsackBoundOutputDouble : public KnapsackBoundOutput<int, double> {
 public:
  virtual void Output(value_type value) {
    std::printf("%g", value);
  }

  virtual void OutputDetail(size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      std::printf("%s%d", sep, weight);
    } else {
      std::printf("%s%d(%g)", sep, weight, value_[index]);
    }
    sep = " ";
  }
};

class KnapsackBoundOutputInt : public KnapsackBoundOutput<int, int> {
 public:
  virtual void Output(value_type value) {
    std::printf("%d", value);
  }

  virtual void OutputDetail(size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      std::printf("%s%d", sep, weight);
    } else {
      std::printf("%s%d(%d)", sep, weight, value_[index]);
    }
    sep = " ";
  }
};

class KnapsackUnboundOutputDouble
    : public KnapsackUnboundOutput<int, double, int> {
 public:
  virtual void Output(value_type value) {
    std::printf("%g", value);
  }

  virtual void OutputDetail(count_type count, size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      std::printf("%s%d*%d=%d", sep, count, weight, count * weight);
    } else {
      value_type value = value_[index];
      std::printf("%s%d*%d(%d*%g=%g)", sep, count, weight, count, value,
        count * value);
    }
    sep = " ";
  }
};

class KnapsackUnboundOutputInt
    : public KnapsackUnboundOutput<int, int, int> {
 public:
  virtual void Output(value_type value) {
    std::printf("%d", value);
  }

  virtual void OutputDetail(count_type count, size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      std::printf("%s%d*%d=%d", sep, count, weight, count * weight);
    } else {
      value_type value = value_[index];
      std::printf("%s%d*%d(%d*%d=%d)", sep, count, weight, count, value,
        count * value);
    }
    sep = " ";
  }
};

#endif  // SRC_KNAPSACK_OUTPUT_H_
