// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_KNAPSACK_H_
#define SRC_KNAPSACK_H_ 1

#include <cassert>  // assert

#include <unordered_map>
#include <vector>

// This is a generic knapsack solver library.
// To use it, inherit a class from
// KnapsackBound<Weight, Value>  or  KnapsackBound<Weight, Value, Count>
// and overload the Output() and OutputDetail() member functions.
// Then initialize max_weight_, the list weight_, and possibly value_.
// Finally, call set_value_is_weight() with correspnding argument and Solver().

template <class Weight> class KnapsackBase {
 public:
  typedef Weight weight_type;
  typedef std::vector<Weight> WeightList;
  typedef typename std::vector<Weight>::size_type size_type;
  WeightList weight_;
  Weight max_weight_;

  virtual ~KnapsackBase() {
  }

  size_type size() {
    return weight_.size();
  }

  virtual void Solve() = 0;

  // This must be called before calling Solve()
  virtual void set_value_is_weight(bool value_is_weight) = 0;
};

template <class Weight, class Value> class Knapsack :
  public KnapsackBase<Weight> {
 private:
  enum ValueIsWeight {
    kUnknown,
    kTrue,
    kFalse
  };
  ValueIsWeight value_is_weight_;

 public:
  typedef KnapsackBase<Weight> super;
  using typename super::size_type;
  using super::weight_;

  typedef Value value_type;
  typedef std::vector<Value> ValueList;
  ValueList value_;

  void set_value_is_weight(bool value_is_weight) {
    if (!value_is_weight) {
      assert(weight_.size() == value_.size());
      value_is_weight_ = kFalse;
      return;
    }
    value_is_weight_ = kTrue;
    value_ = ValueList(weight_.size(), 0);
    for (size_type i = 0; i < weight_.size(); ++i) {
      value_[i] = weight_[i];
    }
  }

  bool get_value_is_weight() {
    assert(value_is_weight_ != kUnknown);
    return (value_is_weight_ == kTrue);
  }
};

template <class Value> class MaxSelected {
 public:
  Value max_value_;
  enum State {
    kUnknown,
    kSelected,
    kUnselected
  };
  State state_;
  MaxSelected() : state_(kUnknown) {
  }
};

template <class Weight, class Value> class KnapsackBound
    : public Knapsack<Weight, Value> {
 public:
  typedef Knapsack<Weight, Value> super;
  using typename super::size_type;
  using super::weight_;
  using super::value_;
  using super::max_weight_;

 private:
  typedef std::unordered_map<Weight, MaxSelected<Value> > WeightValueMap;
  typedef std::vector<WeightValueMap> ItemMap;

  ItemMap hash_map_;

  // Returns max using only i...n, not exceeding max_weight.
  Value SolveRecurse(size_type i, Weight max_weight) {
    if (i == super::size()) {
      return 0;
    }
    MaxSelected<Value>& entry = hash_map_[i][max_weight];
    if (entry.state_ != MaxSelected<Value>::kUnknown) {
      return entry.max_value_;
    }
    Value result0 = SolveRecurse(i + 1, max_weight);
    max_weight -= weight_[i];
    if (max_weight >= 0) {
      Value result1 = value_[i] + SolveRecurse(i + 1, max_weight);
      if (result1 > result0) {
        entry.state_ = MaxSelected<Value>::kSelected;
        return (entry.max_value_ = result1);
      }
    }
    entry.state_ = MaxSelected<Value>::kUnselected;
    return (entry.max_value_ = result0);
  }

  virtual void Output(Value value) = 0;
  virtual void OutputDetail(size_type index) = 0;

 public:
  void Solve() {
    assert(super::value_is_weight_ != super::kUnknown);
    hash_map_ = ItemMap(super::size(), WeightValueMap());
    Output(SolveRecurse(0, max_weight_));
    Weight weight = max_weight_;
    for (size_type i = 0; i < super::size(); ++i) {
      if (hash_map_[i][weight].state_ == MaxSelected<Value>::kSelected) {
        OutputDetail(i);
        weight -= weight_[i];
      }
    }
  }
};

template <class Value, class Count, class size_type> class MaxCount {
 public:
  Value max_value_;
  size_type index_;
  Count count_;
  MaxCount() : count_(0) {
  }
};

template <class Weight, class Value, class Count> class KnapsackUnbound
    : public Knapsack<Weight, Value> {
 public:
  typedef Knapsack<Weight, Value> super;
  using typename super::size_type;
  using typename super::weight_type;
  using typename super::value_type;
  using typename super::WeightList;
  using typename super::ValueList;
  using super::weight_;
  using super::value_;
  using super::max_weight_;

 private:
  typedef std::unordered_map<Weight,
    MaxCount<Value, Count, size_type> > WeightValueMap;

  WeightValueMap hash_map_;

  // Returns max using only i...n, not exceeding max_weight.
  Value SolveRecurse(size_type i, Weight max_weight) {
    if (i == super::size()) {
      return 0;
    }
    MaxCount<Value, Count, size_type>& entry = hash_map_[max_weight];
    if (entry.count_ > 0) {
      return entry.max_value_;
    }
    Weight weight = weight_[i];
    Value value = value_[i];
    Value value_sum = 0;
    Value result = 0;
    Count count(0);
    for (Count c(0); ; ++c) {
      Value r = value_sum + SolveRecurse(i + 1, max_weight);
      if (r > result) {
        result = r;
        count = c;
      }
      if (weight > max_weight) {
        break;
      }
      max_weight -= weight;
      value_sum += value;
    }
    if (count > 0) {
      entry.index_ = i;
      entry.count_ = count;
    }
    return (entry.max_value_ = result);
  }

  virtual void Output(Value value) = 0;
  virtual void OutputDetail(Count count, size_type index) = 0;

 public:
  typedef Count count_type;

  void Solve() {
    assert(super::value_is_weight_ != super::kUnknown);
    hash_map_ = WeightValueMap();
    Output(SolveRecurse(0, max_weight_));
    for (Weight weight = max_weight_; ; ) {
      Count count = hash_map_[weight].count_;
      if (count == 0) {
        break;
      }
      size_type i = hash_map_[weight].index_;
      OutputDetail(count, i);
      weight -= count * weight_[i];
    }
  }
};

#endif  // SRC_KNAPSACK_H_
