// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef KNAPSACK_KNAPSACK_H_
#define KNAPSACK_KNAPSACK_H_ 1

#include <boost/format.hpp>  // boost::format
#include <boost/functional/hash.hpp>  // boost::hash_value

#include <cstdlib>  // std::size_t

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class KnapsackBase {
 public:
  virtual ~KnapsackBase() {
  }

  virtual void SolveAppend(std::string *str) const = 0;
};

template <class Weight, class Count = typename std::vector<Weight>::size_type>
class KnapsackWeight : public KnapsackBase {
 public:
  typedef Weight weight_type;
  typedef std::vector<weight_type> WeightList;
  typedef typename WeightList::size_type size_type;

  WeightList weight_, knapsack_;
  typedef Count count_type;
  typedef typename std::vector<count_type> CountList;
  CountList count_;

  bool empty() const {
    return weight_.empty();
  }

  size_type size() const {
    return weight_.size();
  }

  bool sack_empty() const {
    return knapsack_.empty();
  }

  size_type sack_size() const {
    return knapsack_.size();
  }

  bool IsBound(size_type index) const {
    return ((index >= count_.size()) || (count_[index] > 0));
  }

  count_type get_count(size_type index) const {
    if (index >= count_.size()) {
      return 1;
    }
    return count_[index];
  }

  virtual void clear() {
    weight_ = knapsack_ = WeightList();
    count_ = CountList();
  }
};

template <class Weight, class Value,
    class Count = typename std::vector<Weight>::size_type> class Knapsack
  : public KnapsackWeight<Weight, Count> {
 public:
  typedef KnapsackWeight<Weight, Count> super;
  using typename super::weight_type;
  using typename super::size_type;
  using typename super::count_type;
  using super::weight_;
  using super::knapsack_;

  typedef Value value_type;
  typedef std::vector<value_type> ValueList;
  ValueList value_;

  bool ValueIsWeight(size_type index) const {
    return ((index >= value_.size()) || (value_[index] <= 0));
  }

  value_type get_value(size_type index) const {
    if (ValueIsWeight(index)) {
      return static_cast<value_type>(weight_[index]);
    }
    return value_[index];
  }

  virtual void clear() {
    value_ = ValueList();
    super::clear();
  }

 private:
  // A class which handles a vector as a multiset for hashing and comparison
  class SackSet {
   public:
    typedef typename std::multiset<Weight> WeightSet;
    typedef typename std::vector<Weight> WeightList;
    typedef typename KnapsackWeight<Weight, Count>::size_type size_type;

   private:
    typedef typename WeightSet::iterator iterator;
    typedef typename std::vector<iterator> IteratorList;

    WeightSet set_;
    IteratorList iterator_;

   public:
    SackSet() {
    }

    explicit SackSet(const SackSet& s) {
      assign(s);
    }

    SackSet& operator=(const SackSet& s) {
      assign(s);
      return *this;
    }

    explicit SackSet(const WeightList& weight_list) {
      assign(weight_list);
    }

    void assign(const SackSet& s) {
      set_ = WeightSet();
      iterator_ = IteratorList(s.set_.size(), set_.end());
      for (size_type i(0); i < s.set_.size(); ++i) {
        iterator_[i] = set_.insert(s.get(i));
      }
    }

    void assign(const WeightList& weight_list) {
      set_ = WeightSet();
      iterator_ = IteratorList(weight_list.size(), set_.end());
      for (size_type i(0); i < weight_list.size(); ++i) {
        iterator_[i] = set_.insert(weight_list[i]);
      }
    }

    // Provide a Hash function
    std::size_t operator()(const SackSet& weight_set) const {
      return boost::hash_value(weight_set.set_);
    }

    // Provide the equal operator needed for hashing
    bool operator==(const SackSet &s) const {
      return set_ == s.set_;
    }

    bool operator!=(const SackSet &s) const {
      return set_ != s.set_;
    }

    Weight get(size_type index) const {
      return *iterator_[index];
    }

    // It is the callers responsibility to ensure that no underflow occurs
    void DecreaseBy(size_type index, Weight subtract) {
      iterator& it = iterator_[index];
      Weight old_weight(*it);
      set_.erase(it);
      it = set_.insert(static_cast<Weight>(old_weight - subtract));
    }

    void DecreaseTo(size_type index, Weight new_weight) {
      iterator& it = iterator_[index];
      set_.erase(it);
      it = set_.insert(static_cast<Weight>(new_weight));
    }

    // A separate function for possibly optimizing if multiset<Weight>
    // should one day obtain separate increase/decrease optimizations
    void IncreaseTo(size_type index, Weight new_weight) {
      DecreaseTo(index, new_weight);
    }
  };

  // A class which handles our hashing of bound resources
  class Bound {
   public:
    typedef typename KnapsackWeight<Weight, Count>::size_type size_type;
    typedef Count count_type;

    size_type index_;
    size_type sack_;
    count_type count_;
    SackSet sack_set_;

    Bound() {
    }

    Bound(size_type index, size_type sack, count_type count,
        const SackSet& sack_set)
      : index_(index), sack_(sack), count_(count), sack_set_(sack_set) {
    }

    void assign(size_type index, size_type sack, count_type count,
        const SackSet& sack_set) {
      index_ = index;
      sack_ = sack;
      count_ = count;
      sack_set_ = sack_set;
    }

    std::size_t operator()(const Bound& b) const {
      std::size_t seed(sack_set_(b.sack_set_));
      boost::hash_combine(seed, b.index_);
      boost::hash_combine(seed, b.sack_);
      boost::hash_combine(seed, b.count_);
      return seed;
    }

    bool operator==(const Bound& s) const {
      return ((index_ == s.index_) && (sack_ == s.sack_) &&
        (count_ == s.count_) && (sack_set_ == s.sack_set_));
    }

    bool operator!=(const Bound& s) const {
      return !(this == s);
    }
  };

  class EntryBound {
   public:
    typedef Value value_type;

    bool IsSelected() const {
      return selected_;
    }

    value_type get_value() const {
      return value_;
    }

    void set_value(Value value) {
      value_ = value;
    }

    void Select(Value value) {
      selected_ = true;
      value_ = value;
    }

    EntryBound()
      : selected_(false) {
    }

    explicit EntryBound(Value value)
      : selected_(false), value_(value) {
    }

   private:
    bool selected_;
    value_type value_;
  };

  class EntryUnbound {
   public:
    typedef typename KnapsackWeight<Value, Count>::size_type size_type;
    typedef Value value_type;

    bool IsSelected() const {
      return (sack_ != 0);
    }

    size_type get_item() const {
      return item_;
    }

    size_type get_sack() const {
      return static_cast<size_type>(sack_ - 1);
    }

    value_type get_value() const {
      return value_;
    }

    void Select(size_type item, size_type sack, value_type value) {
      item_ = item;
      sack_ = sack + 1;
      value_ = value;
    }

    EntryUnbound() {
    }

    explicit EntryUnbound(value_type value)
      : sack_(0), value_(value) {
    }

   private:
    size_type item_;
    size_type sack_;
    value_type value_;
  };

  // A class containing all data needed only temporary for calculation.
  class Calc {
   public:
    typedef typename SackSet::WeightList WeightList;
    typedef typename std::unordered_map<Bound, EntryBound, Bound> BoundHash;
    typedef typename std::unordered_map<SackSet, EntryUnbound, SackSet>
      UnboundHash;
    typedef typename KnapsackWeight<Value, Count>::size_type size_type;
    typedef Count count_type;

    SackSet sack_set_;
    BoundHash bound_hash_;
    UnboundHash unbound_hash_;
    bool have_bound_;
    size_type first_bound_;
    count_type first_bound_count_;

    explicit Calc(const WeightList& weight_list)
      : sack_set_(weight_list), have_bound_(false) {
    }

    void SetBound(size_type first_bound, count_type first_bound_count) {
      have_bound_ = true;
      first_bound_ = first_bound;
      first_bound_count_ = first_bound_count;
    }
  };

  mutable Calc *calc_;

  // Returns the first bound item index starting at i (possibly end of list)
  size_type FirstBound(size_type i) const {
    for (; i != super::size() ; ++i) {
      if (super::IsBound(i)) {
        return i;
      }
    }
    return i;
  }

  // Returns max according to current sack_set_,
  // assuming no bound item has been used.
  // It is assumed that super::size() and super::sack_size() are at least 1
  value_type SolveUnbound() const {
    SackSet& sack_set = calc_->sack_set_;
    typename Calc::UnboundHash& hash = calc_->unbound_hash_;

    // Return cached result if possible
    const typename Calc::UnboundHash::const_iterator
      found(hash.find(sack_set));
    if (found != hash.end()) {
      return found->second.get_value();
    }

    // First try without using any item
    EntryUnbound entry(calc_->have_bound_ ?
      SolveBound(calc_->first_bound_, calc_->first_bound_count_, 0) : 0);

    // Place each unbound item into each sack and recurse
    for (size_type i(super::size() - 1); ; --i) {
      if (!super::IsBound(i)) {
        weight_type weight(weight_[i]);
        value_type value(get_value(i));
        for (size_type k(super::sack_size() - 1); ; --k) {
          weight_type sackmax(sack_set.get(k));
          if (weight <= sackmax) {
            sack_set.DecreaseTo(k, sackmax - weight);
            value_type new_value(SolveUnbound() + value);
            sack_set.IncreaseTo(k, sackmax);
            if (new_value > entry.get_value()) {
              entry.Select(i, k, new_value);
            }
          }
          if (k == 0) {
            break;
          }
        }
      }
      if (i == 0) {
        break;
      }
    }

    // Cache result
    hash[sack_set] = entry;
    return entry.get_value();
  }

  // Returns max using only bound items i, i+1, ...
  // using item i at most c times for sack k, k+1, ...
  // according to current sack_set_
  // i must be the index of a bound item
  // k must be the index of a sack
  // c must be positive
  value_type SolveBound(size_type i, count_type c, size_type k) const {
    SackSet& sack_set = calc_->sack_set_;
    typename Calc::BoundHash& hash = calc_->bound_hash_;

    // Return cached result if possible
    Bound bound(i, k, c, sack_set);
    const typename Calc::BoundHash::const_iterator found(hash.find(bound));
    if (found != hash.end()) {
      return found->second.get_value();
    }

    // First try without using item i at sack k
    EntryBound entry;
    {
      size_type tmp(k + 1);
      if (tmp != super::sack_size()) {
        entry.set_value(SolveBound(i, c, tmp));
      } else if ((tmp = FirstBound(i + 1)) != super::size()) {
        entry.set_value(SolveBound(tmp, super::get_count(tmp), 0));
      } else {
        entry.set_value(0);  // No recursion
      }
    }

    // Try with item i at sack k and recurse
    weight_type weight(weight_[i]);
    weight_type sackmax(sack_set.get(k));
    if (weight <= sackmax) {
      value_type new_value(get_value(i));
      bool recurse(true);
      size_type recurse_i(i);
      size_type recurse_k(k);
      count_type recurse_c(c - 1);
      if (recurse_c == 0) {
        recurse_i = FirstBound(i + 1);
        if (recurse_i != super::size()) {
          recurse_k = 0;
          recurse_c = super::get_count(recurse_i);
        } else {
          recurse = false;  // No recursion
        }
      }
      if (recurse) {
        sack_set.DecreaseTo(k, sackmax - weight);
        new_value += SolveBound(recurse_i, recurse_c, recurse_k);
        sack_set.IncreaseTo(k, sackmax);
      }
      if (new_value > entry.get_value()) {
        entry.Select(new_value);
      }
    }

    // Cache result
    hash[bound] = entry;
    return entry.get_value();
  }

 public:
  typedef std::map<size_type, count_type> SackContent;
  typedef std::vector<SackContent> SackList;

  value_type Solve(SackList *sack_list = 0) const {
    if (sack_list) {
      *sack_list = SackList(super::sack_size(), SackContent());
    }
    if (super::empty() || super::sack_empty()) {
      return 0;
    }
    calc_ = new Calc(knapsack_);
    {
      size_type i(FirstBound(0));
      if (i != super::size()) {
        calc_->SetBound(i, super::get_count(i));
      }
    }
    value_type result(SolveUnbound());
    if (sack_list) {
      SackSet& sack_set = calc_->sack_set_;
      {  // Proceed in the order we stored the data: First the unbound items
        typename Calc::UnboundHash& hash = calc_->unbound_hash_;
        for (;;) {
          typename Calc::UnboundHash::const_iterator
            found(hash.find(sack_set));
          if ((found == hash.end()) || !found->second.IsSelected()) {
            break;
          }
          const EntryUnbound& entry = found->second;
          size_type i(entry.get_item());
          size_type k(entry.get_sack());
          ++(*sack_list)[k][i];
          sack_set.DecreaseBy(k, weight_[i]);
        }
      }
      if (calc_->have_bound_) {  // Now the bound items
        size_type i(calc_->first_bound_);
        size_type c(calc_->first_bound_count_);
        size_type k(0);
        typename Calc::BoundHash& hash = calc_->bound_hash_;
        for (;;) {
          typename Calc::BoundHash::const_iterator found(
            hash.find(Bound(i, k, c, sack_set)));
          if ((found == hash.end()) || !found->second.IsSelected()) {
            if (++k != super::sack_size()) {
              continue;
            }
            if ((i = FirstBound(i + 1)) == super::size()) {
              break;
            }
            k = 0;
            c = super::get_count(i);
            continue;
          }
          ++(*sack_list)[k][i];
          sack_set.DecreaseBy(k, weight_[i]);
          if (--c != 0) {
            continue;
          }
          if ((i == FirstBound(i + 1)) == super::size()) {
            break;
          }
          k = 0;
          c = super::get_count(i);
        }
      }
    }
    delete calc_;
    return result;
  }

  virtual boost::format FormatResult() const {
    return boost::format("%s\n%s");
  }

  virtual boost::format FormatKnapsackWeight() const {
    return boost::format("%s|%s: %s\n");
  }

  virtual boost::format FormatKnapsackValue() const {
    return boost::format("%s(%s)|%s: %s\n");
  }

  virtual boost::format FormatSimpleWeight() const {
    return boost::format("%s%s");
  }

  virtual boost::format FormatSimpleValue() const {
    return boost::format("%s%s(%s)");
  }

  virtual boost::format FormatMultiWeight() const {
    return boost::format("%s%s*%s=%s");
  }

  virtual boost::format FormatMultiValue() const {
    return boost::format("%1%%2%*%3%=%4%(%2%*%5%=%6%)");
  }

  virtual const char *FormatInitialSeparator() const {
    return "";
  }

  virtual const char *FormatSeparator() const {
    return " ";
  }

  virtual void SolveAppend(std::string *str) const {
    SackList sack_list;
    value_type result(Solve(&sack_list));
    boost::format
      simple_weight(FormatSimpleWeight()),
      simple_value(FormatSimpleValue()),
      multi_weight(FormatMultiWeight()),
      multi_value(FormatMultiValue()),
      knapsack_weight(FormatKnapsackWeight()),
      knapsack_value(FormatKnapsackValue());
    std::string sacks;
    for (size_type i(0); i < sack_list.size(); ++i) {
      const SackContent& sack = sack_list[i];
      if (sack.empty()) {
        continue;
      }
      weight_type used_weight(0);
      value_type achieved_value(0);
      bool weight_value_equal(true);
      std::string content;
      const char *sep = FormatInitialSeparator();
      for (typename SackContent::const_iterator it(sack.begin());
        it != sack.end(); ++it) {
        size_type item(it->first);
        weight_type weight(weight_[item]);
        count_type count(it->second);
        if (ValueIsWeight(item)) {
          if (count == 1) {
            used_weight += weight;
            achieved_value += static_cast<value_type>(weight);
            content.append((simple_weight % sep % weight).str());
          } else {
            weight_type total_weight(count * weight);
            used_weight += total_weight;
            achieved_value += static_cast<value_type>(total_weight);
            content.append((multi_weight % sep % count % weight %
              total_weight).str());
          }
        } else {
          weight_value_equal = false;
          value_type value(value_[item]);
          if (count == 1) {
            used_weight += weight;
            achieved_value += value;
            content.append((simple_value % sep % weight % value).str());
          } else {
            weight_type total_weight(count * weight);
            used_weight += total_weight;
            value_type total_value(count * value);
            achieved_value += total_value;
            content.append((multi_value % sep % count % weight %
              total_weight % value % total_value).str());
          }
        }
        sep = FormatSeparator();
      }
      if (weight_value_equal) {
        sacks.append((knapsack_weight % used_weight % knapsack_[i] %
          content).str());
      } else {
        sacks.append((knapsack_value % used_weight % achieved_value %
          knapsack_[i] % content).str());
      }
    }
    str->append((FormatResult() % result % sacks).str());
  }
};

#endif  // KNAPSACK_KNAPSACK_H_
