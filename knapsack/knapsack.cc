// This file is part of the knapsack project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "knapsack/knapsack.h"

#include <boost/algorithm/string.hpp>  // boost::split, boost::is_any_of
#include <boost/format.hpp>  // boost::format

#include <unistd.h>  // getopt

#include <cctype>  // isdigit
#include <cstdlib>  // exit
#include <cstdio>  // puts, fputs, stderr, stdout

#include <iostream>  // istringstream
#include <string>
#include <vector>


static const char *version = "knapsack 4.0";

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

ATTRIBUTE_NORETURN inline static void Die(const string& s);
ATTRIBUTE_NORETURN inline static void Die(const boost::format& s);
ATTRIBUTE_NORETURN static void Die(const char *s);
inline static Integer ParseNumber(const string& s, bool allow_zero = false);
static void Help();
static Integer ParseNumber(const char *s, bool allow_zero = false);
static KnapsackCommon *ParseOpts(int argc, char *argv[], bool *floating_point);

static void Help() {
  std::puts("Usage: knapsack [options] "
"-s [count*]sack [-s [count*]sack ...] item item ...\n"
"\n"
"A sack is a positive integer number describing how much weight it can carry.\n"
"If a positive integer number count is specified, this is a short form for\n"
" -s sack -s sack .... (count times)\n"
"\n"
"An item has the form [N*]weight[=value]\n"
"The weight must be a positive integer, and N must be a nonnegative integer.\n"
"If value is not specified it is assumed to be the same as the weight.\n"
"A positive integer N means that the item is available N times.\n"
"The integer N=0 is interpreted as infinity, that is, there is an unbound\n"
"amount of corresponding items available. It would be possible to specify a\n"
"sufficiently large number instead of N=0, but the case N=0 can be handled\n"
"more efficiently concerning space and time requirements.\n"
"\n"
"For convenience (to avoid quoting for shells), in the specifiaction of"
"sacks\n"
"and items, one can instead of the separating symbol * also used a space\n"
"(or tab, linefeed, or newline) or x, X, or a colon (:).\n"
"Similarly, instead of the = symbol in items, one can use ~, #, or @.\n"
"\n"
"Options:\n"
"-f values are floating point numbers\n"
"-V output the version number and exit\n"
"-h output this text and exit");
}

inline static void Die(const std::string& s) {
  Die(s.c_str());
}

inline static void Die(const boost::format& s) {
  Die(s.str());
}

inline static Integer ParseNumber(const string& s, bool allow_zero) {
  return ParseNumber(s.c_str(), allow_zero);
}

int main(int argc, char *argv[]) {
  bool floating_point;
  KnapsackCommon *knapsack(ParseOpts(argc, argv, &floating_point));
  if (optind >= argc) {
    Die("no items specified");
  }
  for (int i(optind); i < argc; ++i) {
    vector<string> parts;
    boost::split(parts, argv[i], boost::is_any_of(":*xX \t\r\n"));
    string rest;
    if (parts.size() <= 1) {
      rest = parts[0];
      knapsack->count_.push_back(1);
    } else {
      rest = parts[1];
      knapsack->count_.push_back(ParseNumber(parts[0], true));
    }
    boost::split(parts, rest, boost::is_any_of("=~#@"));
    knapsack->weight_.push_back(ParseNumber(parts[0]));
    if (parts.size() <= 1) {
      if (floating_point) {
        static_cast<KnapsackFloat *>(knapsack)->value_.push_back(0);
      } else {
        static_cast<KnapsackInt *>(knapsack)->value_.push_back(0);
      }
    } else {
      if (floating_point) {
        std::istringstream is(parts[1]);
        Float value;
        is >> value;
        if (value <= 0) {
          Die(boost::format("value is not positive: %s") % value);
        }
        static_cast<KnapsackFloat *>(knapsack)->value_.push_back(value);
      } else {
        static_cast<KnapsackInt *>(knapsack)->value_.push_back(
          ParseNumber(parts[1]));
      }
    }
  }
  string result;
  knapsack->SolveAppend(&result);
  delete knapsack;
  fputs(result.c_str(), stdout);
  return 0;
}

KnapsackCommon *ParseOpts(int argc, char *argv[], bool *floating_point) {
  *floating_point = false;
  vector<Integer> sacks;
  int opt;
  while ((opt = getopt(argc, argv, "fs:Vh")) != -1) {
    switch (opt) {
      case 'f':
        *floating_point = true;
        break;
      case 's':
        {
          vector<string> parts;
          boost::split(parts, optarg, boost::is_any_of(":*xX"));
          if (parts.size() <= 1) {
            sacks.push_back(ParseNumber(parts[0]));
          } else {
            sacks.insert(sacks.end(), ParseNumber(parts[0]),
              ParseNumber(parts[1]));
          }
        }
        break;
      case 'V':
        std::puts(version);
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
  if (sacks.empty()) {
    Die("no knapsack specified (with option -s)");
  }
  KnapsackCommon *knapsack;
  if (*floating_point) {
    knapsack = new KnapsackFloat;
  } else {
    knapsack = new KnapsackInt;
  }
  knapsack->knapsack_ = sacks;
  return knapsack;
}

static Integer ParseNumber(const char *s, bool allow_zero) {
  const char *ori(s);
  Integer weight = 0;
  if (*s == '+') {
    ++s;
  }
  for (char c(*s); std::isdigit(c); c = *(++s)) {
    Integer new_weight = (weight * 10) + static_cast<Integer>(c - '0');
    if (new_weight <= weight) {
      if ((weight != 0) || (c != '0')) {
        Die(boost::format("number overflow in %s") % ori);
      }
    }
    weight = new_weight;
  }
  if ((!allow_zero) && (weight == 0)) {
    Die(boost::format("not a positive integer: %s") % ori);
  }
  return weight;
}

static void Die(const char *s) {
  fputs(s, stderr);
  fputs("\nType knapsack -h for help\n", stderr);
  std::exit(EXIT_FAILURE);
}
