# knapsack

A fast solver for the 0/1-knapsack problem with multiple knapsacks

(C) Martin Väth (martin at mvath.de)-
This project is distributed under the terms of the
GNU General Public License v2.
SPDX-License-Identifier: GPL-2.0-only

This program was written rather quickly to train myself.
It was later rewritten in a more general and efficient way.

It uses a dynamic programming type approach to the 0/1 knapsack problem
(in the bound or unbound form) for multiple knapsacks.

In practice, one typically runs into this problem if one wants to distribute
files of certain sizes to e.g. one or several USB-Sticks or CD-Roms:
One is looking for a distribution of the files onto the media (knapsack)
such that the amount of data is maximized.
In some cases, the value of the data is differs from its filesize.
Therefore, it is convenient that one can give values independently of the size.
For example, if one gives some extremely high values to some files, one makes
sure that the solution eventually found contains at least these files.

Some notes on the algorithm:
The classical dynamic programming approach based on arrays
requires an enormous amount of memory if the weights are huge:
Especially for the above mentioned application, this classical approach
would need more memory for calculation than what matches on all considered
media togethher.

In contrast, the algorithm of this library uses hashing and a recursive
approach to calculate less data and require less memory, in general.
There are some cases (e.g. if you have an unlimited number of files of size
1 bytes to distribute) in which also this algorithm will require too much
memory, but in "typical" cases it should be relative fast and low in memory
consumption. After all, the full problem is NP-complete, so by current state
of research, one cannot expect a "perfect" solution.

The program consists of a library which can be used independently:

- `knapsack.h`:
	The main generic class implementing the algorithm.
	The output functions have to be overloaded.

The library is consumed by:

- `knapsack.cc`:
	A simple CLI to the library

## Requirements

The library needs the boost library for advanced hash functions.
Since boost is needed anyway, also `boost::format` is used for convenience
(the CLI additionally uses `boost::program_options`, `boost::lexical_cast`,
and `boost::split`).

## Installation

If the requirement is fulfilled, compilation of the CLI should be simple:
Just run `make` (as user) and `make install` (as root)
Look at the `Makefile` if you need to adapt paths.

To use the libraries, just include them (with the appropriate path);
the description how to use the library is in the file `knapsack.txt`.

To use the zsh completion file, make sure that it is in you zsh's `$fpath`.

## Usage

To get help for the CLI, type
```
	knapsack -h
```
