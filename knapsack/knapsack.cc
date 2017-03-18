// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "knapsack/knapsack.h"

#include <boost/program_options.hpp>  // boost::program_options
#include <boost/algorithm/string.hpp>  // boost::split, boost::is_any_of
#include <boost/format.hpp>  // boost::format
#include <boost/lexical_cast.hpp>  // boost::lexical_cast

#include <unistd.h>  // getopt

#include <cstdlib>  // exit
#include <cstdio>  // puts, fputs, stderr, stdout

#include <string>
#include <vector>


static const char *version = "knapsack 7.1";

using std::string;
using std::vector;

// Use largest unsigned integers/floats available:

typedef long double Float;

#if __cplusplus >= 201103L
typedef unsigned long long Integer;  // NOLINT(runtime/int)
#define ATTRIBUTE_NORETURN [[noreturn]]  // NOLINT(whitespace/braces)
#else
typedef unsigned long Integer;  // NOLINT(runtime/int)
#define ATTRIBUTE_NORETURN
#endif

typedef KnapsackWeight<Integer, Integer> KnapsackCommon;
typedef Knapsack<Integer, Integer, Integer> KnapsackInt;
typedef Knapsack<Integer, Float, Integer> KnapsackFloat;

typedef vector<string> WordList;

static void Help(const boost::program_options::options_description& options);
template<class T> void Warn(T s);
template<class T> ATTRIBUTE_NORETURN void Die(T s);
template<class T> T ParseNumber(const string& s, bool check_positive = true);
static Integer CountMax(const KnapsackCommon& sack, Integer weight);
KnapsackCommon *opt_parse(int argc, char *argv[]);

static void Help(const boost::program_options::options_description& options) {
  std::puts((boost::format("Usage: knapsack [options] [item item ...]\n"
"The input must specify items (by specifying their weight and value, and how\n"
"often each item is available) and one or several knapsacks (by specifying\n"
"the weight which each knapsack can carry).\n"
"The output describes how the items can be placed into the knapsacks such\n"
"that no knapsack contains more than the specified weight of items and such\n"
"that the total values of all items in the knapsack is maximal.\n"
"\n"
"Note that even for a single knapsack, this is a so-called NP hard problem:\n"
"There are instances with even relatively few items where all known\n"
"algorithm can take millions of years for calculating the optimum.\n"
"This program uses a so-called dynamic programming technique to trade memory\n"
"for time. It uses hashing instead of array to minimize the memory.\n"
"It is possible that also this program runs out of time or memory, but for\n"
"relatively small instances, it will usually find a solution.\n"
"If a solution is found, it is optimal (ignoring rounding errors when\n"
"floating point numbers are used which is therefore not recommended).\n"
"\n"
"knapsacks can be specified with option -s, items with option -i or also\n"
"simply as arguments on the command line (see option -i below for details.)\n"
"If there is no item which can be put into a knapsack, the program exits\n"
"with nonzero error status and an error message; otherwise the program prints\n"
"a solution with maximal value and returns with zero error status, unless it\n"
"runs out of memory.\n"
"\n%s\n") % options).str().c_str());
}

template<class T> ATTRIBUTE_NORETURN void Warn(T s) {
  fputs((boost::format("knapsack: warning: %s\n") % s).str().c_str(), stderr);
}

template<class T> ATTRIBUTE_NORETURN void Die(T s) {
  fputs((boost::format("knapsack: %s\nType knapsack -h for help\n") % s)
    .str().c_str(), stderr);
  std::exit(EXIT_FAILURE);
}

template<class T> T ParseNumber(const string& s, bool check_positive) {
  T result;
  try {
    result = boost::lexical_cast<T>(s);
  } catch (const boost::bad_lexical_cast &e) {
    Die(boost::format("number %s: %s") % s % e.what());
  }
  if (check_positive && (result <= 0)) {
    Die(boost::format("number %s: not a positive number") % s);
  }
  return result;
}

static Integer CountMax(const KnapsackCommon& sack, Integer weight) {
  Integer count_max(0);
  const KnapsackCommon::WeightList& sacks = sack.knapsack_;
  for (KnapsackCommon::WeightList::const_iterator it(sacks.begin()),
    end(sacks.end()); it != end; ++it) {
    count_max += (*it) / weight;
  }
  return count_max;
}

KnapsackCommon *opt_parse(int argc, char *argv[]) {
  bool opt_quiet(false), opt_float(false), opt_force(false),
    opt_version(false), opt_help(false);
  WordList opt_sack, opt_item;
  boost::program_options::options_description options("Options");
  options.add_options()
    ("sack,s", boost::program_options::value<WordList>(&opt_sack),
      "specify knapsack capacity: the integer weight the knapsack can carry.\n"
      "If used repeatedly, multiple knapsacks will be used.\n"
      "Instead of N times repeating -s arg, one can use the syntax "
      "-s N*arg (instead of * one can also use : or x or X)")
    ("item,i", boost::program_options::value<WordList>(&opt_item),
      "specify item. An item has has form [N*]weight[=value].\n"
      "It means that there are N items of the specified (integer) weight "
      "and value which can be distributed to the knapsacks. "
      "The value can be a fractional (float) number if option -f is used.\n"
      "If N is omitted, it defaults to 1. If value is omitted or not positive "
      "then the value of the item defaults to its weight.\n"
      "The case N=0 means that the availability of this items is unbound; "
      "this case is treated more efficiently than bound availability. "
      "(This efficient case is automatically selected if N is so large that "
      "the knapsacks cannot carry more than N times this item only.)\n"
      "The symbol * can be replaced by : or x or X, and the symbol = "
      "can be replaced by ~ or # or @.")
    ("float,f", boost::program_options::bool_switch(&opt_float),
      "values of items can be fractional (floating point) numbers.\n"
      "Without this option, all values must be integer numbers. "
      "With this option, the result might be wrong due to (accumulative) "
      "rounding errors which are ignored by the algorithm.")
    ("quiet,q", boost::program_options::bool_switch(&opt_quiet),
      "do not print warnings about ignored items/modified N")
    ("force,F", boost::program_options::bool_switch(&opt_force),
      "use items as specified on the command line, even if they are "
      "too heavy to fit anywhere or if some number could be treated as "
      "unbound more effficiently. This serves mainly for debugging purposes, "
      "but it could also be that a different solution is found with this "
      "option if several optimal solutions do exist")
    ("version,V", boost::program_options::bool_switch(&opt_version),
      "output the version number and exit")
    ("help,h", boost::program_options::bool_switch(&opt_help),
      "output help text and exit");
  boost::program_options::positional_options_description positional;
  positional.add("item", -1);
  boost::program_options::variables_map option_map;
  try {
    boost::program_options::store(boost::program_options::command_line_parser(
        argc, argv).options(options).positional(positional).run(),
      option_map);
    boost::program_options::notify(option_map);
  } catch(std::exception& e) {
    Die(e.what());
  }
  if (opt_version) {
    std::puts(version);
    std::exit(EXIT_SUCCESS);
  }
  if (opt_help) {
    Help(options);
    std::exit(EXIT_SUCCESS);
  }
  KnapsackCommon *knapsack;
  if (opt_float) {
    knapsack = new KnapsackFloat;
  } else {
    knapsack = new KnapsackInt;
  }
  KnapsackCommon::WeightList& sack = knapsack->knapsack_;
  for (WordList::const_iterator it(opt_sack.begin()); it != opt_sack.end();
    ++it) {
    WordList parts;
    boost::split(parts, *it, boost::is_any_of(":*xX"));
    if (parts.size() <= 1) {
      sack.push_back(ParseNumber<Integer>(parts[0]));
    } else {
      sack.insert(sack.end(), ParseNumber<Integer>(parts[0]),
        ParseNumber<Integer>(parts[1]));
    }
  }
  if (knapsack->sack_empty()) {
    Die("at least one knapsack must be specified, e.g. with option -s");
  }
  for (WordList::const_iterator it(opt_item.begin()); it != opt_item.end();
    ++it) {
    WordList parts;
    boost::split(parts, *it, boost::is_any_of(":*xX \t\r\n"));
    string rest;
    Integer count;
    if (parts.size() <= 1) {
      rest = parts[0];
      count = 1;
    } else {
      rest = parts[1];
      count = ParseNumber<Integer>(parts[0], false);
    }
    boost::split(parts, rest, boost::is_any_of("=~#@"));
    Integer weight(ParseNumber<Integer>(parts[0]));
    if (!opt_force) {
      Integer count_max(CountMax(*knapsack, weight));
      if (count_max <= 0) {
        if (!opt_quiet) {
          Warn(boost::format("ignoring too heavy item %s") % *it);
        }
        continue;
      }
      if (count >= count_max) {
        if (!opt_quiet) {
          Warn(boost::format("treating item %s as unbound (N=0) "
            "for efficiency") % *it);
        }
        count = 0;
      }
    }
    knapsack->count_.push_back(count);
    knapsack->weight_.push_back(weight);
    if (parts.size() <= 1) {
      if (opt_float) {
        static_cast<KnapsackFloat *>(knapsack)->value_.push_back(0);
      } else {
        static_cast<KnapsackInt *>(knapsack)->value_.push_back(0);
      }
    } else {
      if (opt_float) {
        static_cast<KnapsackFloat *>(knapsack)->value_.push_back(
          ParseNumber<Float>(parts[1]));
      } else {
        static_cast<KnapsackInt *>(knapsack)->value_.push_back(
          ParseNumber<Integer>(parts[1]));
      }
    }
  }
  if (knapsack->empty()) {
    Die("at least one not too heavy item must be specified");
  }
  return knapsack;
}

int main(int argc, char *argv[]) {
  KnapsackCommon *knapsack(opt_parse(argc, argv));
  string result;
  knapsack->SolveAppend(&result);
  delete knapsack;
  fputs(result.c_str(), stdout);
  return 0;
}
