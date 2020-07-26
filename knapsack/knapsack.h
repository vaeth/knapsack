// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
// SPDX-License-Identifier: GPL-2.0-only
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef KNAPSACK_KNAPSACK_H_
#define KNAPSACK_KNAPSACK_H_ 1

#include <boost/format.hpp>  // boost::format
#include <boost/unordered_map.hpp>  // boost::unordered_map and hash stuff

#include <cstdlib>  // std::size_t

#include <map>
#include <set>
#include <string>
#include <utility>  // std::pair
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
  typedef typename std::multiset<Weight> WeightSet;

  // A class containing all data of bound items:
  // item (first) and its count (second)
  typedef std::pair<size_type, count_type> BoundItem;

  // A class which serves as the hash index for bound items
  class BoundIndex {
   public:
    typedef typename Knapsack<Weight, Value, Count>::BoundItem BoundItem;
    typedef typename KnapsackWeight<Weight, Count>::size_type size_type;
    typedef typename KnapsackWeight<Weight, Count>::count_type count_type;
    typedef typename Knapsack<Weight, Value, Count>::WeightSet WeightSet;

    BoundItem bound_item_;
    WeightSet sack_set_;

    BoundIndex(const BoundItem& bound_item, const WeightSet& sack_set)
      : bound_item_(bound_item), sack_set_(sack_set) {
    }

    void assign(const BoundItem& bound_item, const WeightSet& sack_set) {
      bound_item_ = bound_item;
      sack_set_ = sack_set;
    }

    friend std::size_t hash_value(const BoundIndex& b) {
      std::size_t seed(0);
      boost::hash_combine(seed, b.bound_item_);
      boost::hash_combine(seed, b.sack_set_);
      return seed;
    }

    bool operator==(const BoundIndex& b) const {
      return ((bound_item_ == b.bound_item_) && (sack_set_ == b.sack_set_));
    }

    const BoundItem& get_bound() const {
      return bound_item_;
    }

    size_type get_item() const {
      return bound_item_.first;
    }

    count_type get_count() const {
      return bound_item_.second;
    }
  };

  class EntryBound {
   public:
    typedef typename KnapsackWeight<Weight, Count>::size_type size_type;
    typedef Value value_type;

    bool IsSelected() const {
      return (sack_ != 0);
    }

    size_type get_sack() const {
      return static_cast<size_type>(sack_ - 1);
    }

    value_type get_value() const {
      return value_;
    }

    void set_value(Value value) {
      value_ = value;
    }

    void Select(size_type sack, value_type value) {
      sack_ = sack + 1;
      value_ = value;
    }

    EntryBound()
      : sack_(0) {
    }

    explicit EntryBound(Value value)
      : sack_(0), value_(value) {
    }

   private:
    size_type sack_;
    value_type value_;
  };

  class EntryUnbound : public EntryBound {
   public:
    typedef EntryBound super;
    using typename super::size_type;
    using typename super::value_type;

    size_type get_item() const {
      return item_;
    }

    void Select(size_type item, size_type sack, value_type value) {
      item_ = item;
      super::Select(sack, value);
    }

    EntryUnbound() {
    }

    explicit EntryUnbound(value_type value)
      : super(value) {
    }

   private:
    size_type item_;
  };

  // A class containing all data needed only temporarily for calculation
  class Calc {
   public:
    typedef typename Knapsack<Weight, Value, Count>::WeightSet WeightSet;
    typedef typename boost::unordered_map<BoundIndex, EntryBound> BoundHash;
    typedef typename boost::unordered_map<WeightSet, EntryUnbound> UnboundHash;
    typedef typename KnapsackWeight<Weight, Count>::size_type size_type;
    typedef typename KnapsackWeight<Weight, Count>::count_type count_type;
    typedef typename KnapsackWeight<Weight, Count>::WeightList WeightList;

    WeightSet sack_set_;
    BoundHash bound_hash_;
    UnboundHash unbound_hash_;
    bool have_bound_;
    BoundItem bound_;

   private:
    typedef typename WeightSet::iterator Iterator;
    typedef std::vector<Iterator> IteratorList;
    IteratorList iterator_;

    // This class is only meant to be used statically:
    Calc() {}
    Calc(const Calc&) {}
    Calc& operator=(const Calc&) {}

   public:
    explicit Calc(const WeightList& weight_list)
      : have_bound_(false) {
      iterator_ = IteratorList(weight_list.size(), sack_set_.end());
      typename WeightList::const_iterator sit(weight_list.begin());
      for (typename IteratorList::iterator it(iterator_.begin()),
        it_end(iterator_.end()); it != it_end; ++it, ++sit) {
        *it = sack_set_.insert(*sit);
      }
    }

    void SetBound(size_type item, count_type count) {
      bound_ = BoundItem(item, count);
      have_bound_ = true;
    }

    weight_type SackMax(size_type index) const {
      return *iterator_[index];
    }

    // It is the caller's responsibility to ensure that no underflow occurs
    void DecreaseBy(size_type index, weight_type subtract) {
      Iterator *it_pointer(&(iterator_[index]));
      Iterator it(*it_pointer);
      weight_type new_weight(static_cast<weight_type>(*it - subtract));
      sack_set_.erase(it);
      *it_pointer = sack_set_.insert(new_weight);
    }

    void DecreaseTo(size_type index, weight_type new_weight) {
      Iterator& it_reference = iterator_[index];
      sack_set_.erase(it_reference);
      it_reference = sack_set_.insert(new_weight);
    }

    // A separate function for possibly optimizing a different implementation
    void IncreaseTo(size_type index, weight_type new_weight) {
      DecreaseTo(index, new_weight);
    }
  };

  // Returns the first bound item index starting at item (possibly end of list)
  size_type FirstBound(size_type item) const {
    for (; item != super::size() ; ++item) {
      if (super::IsBound(item)) {
        return item;
      }
    }
    return item;
  }

  // Returns max according to calc->sack_set_,
  // assuming no bound item has been used.
  // It is assumed that super::size() and super::sack_size() are at least 1
  value_type SolveUnbound(Calc *calc) const {
    WeightSet& sack_set = calc->sack_set_;
    typename Calc::UnboundHash& hash = calc->unbound_hash_;

    // Return cached result if possible
    const typename Calc::UnboundHash::const_iterator
      found(hash.find(calc->sack_set_));
    if (found != hash.end()) {
      return found->second.get_value();
    }

    // First try without using any item
    EntryUnbound entry(calc->have_bound_ ? SolveBound(calc) : 0);

    // Place each unbound item into each sack and recurse
    for (size_type item(super::size() - 1); ; --item) {
      if (!super::IsBound(item)) {
        weight_type weight(weight_[item]);
        value_type value(get_value(item));
        for (size_type sack(super::sack_size() - 1); ; --sack) {
          weight_type sackmax(calc->SackMax(sack));
          if (weight <= sackmax) {
            calc->DecreaseTo(sack, sackmax - weight);
            value_type new_value(SolveUnbound(calc) + value);
            calc->IncreaseTo(sack, sackmax);
            if (new_value > entry.get_value()) {
              entry.Select(item, sack, new_value);
            }
          }
          if (sack == 0) {
            break;
          }
        }
      }
      if (item == 0) {
        break;
      }
    }

    // Cache result
    hash[sack_set] = entry;
    return entry.get_value();
  }

  // Returns max using only bound items calc->bound_.first or later,
  // the first item at most calc->bound_.second times,
  // according to current calc->sack_set_.
  // It is assumed that super::sack_size() is at least 1, and moreover:
  // calc->bound_.first must be the index of a bound item, and
  // calc->bound_.second must be positive
  value_type SolveBound(Calc *calc) const {
    typename Calc::BoundHash& hash = calc->bound_hash_;

    // Return cached result if possible
    BoundIndex bound_index(calc->bound_, calc->sack_set_);
    const typename Calc::BoundHash::const_iterator
      found(hash.find(bound_index));
    if (found != hash.end()) {
      return found->second.get_value();
    }

    size_type item(bound_index.get_item());

    // First try without using the first item
    bool recurse;
    {  // prepare calc->bound_ for the next item; set recurse if we have one
      size_type next_item(FirstBound(item + 1));
      if ((recurse = (next_item != super::size()))) {
        calc->bound_ = BoundItem(next_item, super::get_count(next_item));
      }
    }
    EntryBound entry(recurse ? SolveBound(calc) : 0);

    // Try with current item at each sack and recurse
    {  // Use decreased calc->bound_.second for the next recursion if positive
      count_type count(bound_index.get_count() - 1);
      if (count > 0) {
        calc->bound_ = BoundItem(item, count);
        recurse = true;
      }  // else: calc->bound_ already refers to the next item
    }
    weight_type weight(weight_[item]);
    value_type value(get_value(item));
    for (size_type sack(super::sack_size() - 1); ; --sack) {
      weight_type sackmax(calc->SackMax(sack));
      if (weight <= sackmax) {
        if (!recurse) {  // This is the last item which can be inserted
          entry.Select(sack, value);
          break;  // All sacks for the last item are equally good: break loop
        }
        calc->DecreaseTo(sack, sackmax - weight);
        value_type new_value(SolveBound(calc) + value);
        calc->IncreaseTo(sack, sackmax);
        if (new_value > entry.get_value()) {
          entry.Select(sack, new_value);
        }
      }
      if (sack == 0) {
        break;
      }
    }

    // restore data which we possibly changed for recursion
    calc->bound_ = bound_index.get_bound();

    // Cache result
    hash[bound_index] = entry;
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
    Calc calc(knapsack_);
    {
      size_type item(FirstBound(0));
      if (item != super::size()) {
        calc.SetBound(item, super::get_count(item));
      }
    }
    value_type result(SolveUnbound(&calc));
    if (sack_list) {
      typename Calc::BoundHash::const_iterator
        bound_end(calc.bound_hash_.end());
      typename Calc::UnboundHash::const_iterator
        unbound_end(calc.unbound_hash_.end());
      bool have_bound(calc.have_bound_);
      BoundItem bound(calc.bound_);
      // bound and unbound items might be wildly mixed in the hashes:
      // We start with all unbound (since this is likely the first) as long as
      // we find, then all bound as long as we find, then again all unbound
      // (unless bound found nothing), then all bound (starting with next item
      // if unbound found nothing), etc.
      for (bool found_bound(true), found_unbound(true); ;
        found_unbound = false) {
        if (found_bound) {  // bound search succeeded or we are in first round
          for (;;) {  // search/update according to unbound_hash
            typename Calc::UnboundHash::const_iterator
              found(calc.unbound_hash_.find(calc.sack_set_));
            if ((found == unbound_end) || !found->second.IsSelected()) {
              break;
            }
            found_unbound = true;
            const EntryUnbound& entry = found->second;
            size_type item(entry.get_item());
            size_type sack(entry.get_sack());
            ++(*sack_list)[sack][item];
            calc.DecreaseBy(sack, weight_[item]);
          }
        }
        if (!have_bound) {
          break;
        }
        if (!found_unbound) {  // unbound search failed and not in first round
          // Pass to next item
          size_type i(FirstBound(bound.first + 1));
          if (i == super::size()) {  // search for bound items is finished
            break;
          }
          bound = BoundItem(i, super::get_count(i));
        }
        found_bound = false;
        for (;;) {  // search/update according to bound_hash
          typename Calc::BoundHash::const_iterator found(
            calc.bound_hash_.find(BoundIndex(bound, calc.sack_set_)));
          if ((found == bound_end) || !found->second.IsSelected()) {
            break;
          }
          found_bound = true;
          size_type item(bound.first);
          size_type sack(found->second.get_sack());
          ++(*sack_list)[sack][item];
          calc.DecreaseBy(sack, weight_[item]);
          if (--bound.second == 0) {  // Update bound resources
            size_type i(FirstBound(bound.first + 1));
            if (i == super::size()) {  // search for bound items is finished
              have_bound = false;  // still retry once unbound items
              break;
            }
            bound = BoundItem(i, super::get_count(i));
          }
        }
      }
    }
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
