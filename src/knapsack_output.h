// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_KNAPSACK_OUTPUT_H_
#define SRC_KNAPSACK_OUTPUT_H_ 1

// Instances of KnapsackBound/KnapsackUnbound with typesafe output using
// https://github.com/vaeth/osformat

#include <osformat.h>

#include "src/knapsack.h"

template <class Weight, class Value> class KnapsackBoundOutput
    : public KnapsackBound<Weight, Value> {
 protected:
  const char *sep;

 public:
  typedef KnapsackBound<Weight, Value> super;
  using typename super::weight_type;
  using typename super::value_type;
  using typename super::size_type;
  using super::weight_;
  using super::value_;
  using super::get_value_is_weight;

  virtual void Solve() {
    sep = "\n";
    super::Solve();
    osformat::Say(false);
  }

  virtual void Output(Value value) {
    osformat::Print() % value;
  }

  virtual void OutputDetail(size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      osformat::Print("%s%s") % sep % weight;
    } else {
      osformat::Print("%s%s(%s)") % sep % weight % value_[index];
    }
    sep = " ";
  }
};

template <class Weight, class Value, class Count> class KnapsackUnboundOutput
    : public KnapsackUnbound<Weight, Value, Count> {
 protected:
  const char *sep;

 public:
  typedef KnapsackUnbound<Weight, Value, Count> super;
  using typename super::weight_type;
  using typename super::value_type;
  using typename super::count_type;
  using typename super::size_type;
  using super::weight_;
  using super::value_;
  using super::get_value_is_weight;

  virtual void Solve() {
    sep = "\n";
    super::Solve();
    osformat::Say(false);
  }

  virtual void Output(value_type value) {
    osformat::Print() % value;
  }

  virtual void OutputDetail(count_type count, size_type index) {
    weight_type weight = weight_[index];
    if (get_value_is_weight()) {
      osformat::Print("%s%s*%s=%s") % sep % count % weight % (count * weight);
    } else {
      value_type value = value_[index];
      osformat::Print("%s%s*%s(%s*%s=%s)") % sep % count % weight % count
        % value % (count * value);
    }
    sep = " ";
  }
};

#endif  // SRC_KNAPSACK_OUTPUT_H_
