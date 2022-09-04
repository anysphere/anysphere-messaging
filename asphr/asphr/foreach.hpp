//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

/*
Inspired by https://www.scs.stanford.edu/~dm/blog/va-opt.html.

Usage:
* ASPHR_FOR_EACH(F, a, b, c, 1, 2, 3) // => F(a), F(b), F(c), F(1), F(2), F(3)
* ASPHR_FOR_EACH2(F1, F2, a, b, c, 1, 2, 3) // => F1(a), F2(b), F1(c), F2(1),
F1(2), F2(3)
*/

#define ASPHR_PARENS ()

#define ASPHR_EXPAND(...) \
  ASPHR_EXPAND4(ASPHR_EXPAND4(ASPHR_EXPAND4(ASPHR_EXPAND4(__VA_ARGS__))))
#define ASPHR_EXPAND4(...) \
  ASPHR_EXPAND3(ASPHR_EXPAND3(ASPHR_EXPAND3(ASPHR_EXPAND3(__VA_ARGS__))))
#define ASPHR_EXPAND3(...) \
  ASPHR_EXPAND2(ASPHR_EXPAND2(ASPHR_EXPAND2(ASPHR_EXPAND2(__VA_ARGS__))))
#define ASPHR_EXPAND2(...) \
  ASPHR_EXPAND1(ASPHR_EXPAND1(ASPHR_EXPAND1(ASPHR_EXPAND1(__VA_ARGS__))))
#define ASPHR_EXPAND1(...) __VA_ARGS__

#define ASPHR_FOR_EACH(macro, between, ...) \
  __VA_OPT__(ASPHR_EXPAND(ASPHR_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define ASPHR_FOR_EACH_HELPER(macro, a1, ...) \
  macro(a1) __VA_OPT__(, ASPHR_FOR_EACH_AGAIN ASPHR_PARENS(macro, __VA_ARGS__))
#define ASPHR_FOR_EACH_AGAIN() ASPHR_FOR_EACH_HELPER

#define ASPHR_FOR_EACH2(macro1, macro2, ...) \
  __VA_OPT__(ASPHR_EXPAND(ASPHR_FOR_EACH2_HELPER(macro1, macro2, __VA_ARGS__)))
#define ASPHR_FOR_EACH2_HELPER(macro1, macro2, a1, a2, ...)               \
  macro1(a1), macro2(a2) __VA_OPT__(, ASPHR_FOR_EACH2_AGAIN ASPHR_PARENS( \
                                          macro1, macro2, __VA_ARGS__))
#define ASPHR_FOR_EACH2_AGAIN() ASPHR_FOR_EACH2_HELPER
