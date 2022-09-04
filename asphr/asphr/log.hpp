//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <iostream>

#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "foreach.hpp"

/*
ASPHR_LOG is the simplest possible logging mechanism.

Features we want:
* severity levels
* structured logging
* works fine with multiple threads

Features we don't want:
* everything else

Why not use a more standard logging library?
* glog reads environment variables, adds gflags, and adds a lot of other stuff
* log4cxx is a close cousin to log4j..... (need i say more?)
* nanolog requires a separate binary to decode the logfiles

If it is determined that the logging ever causes a performance bottleneck, it
may be worth looking into the above libraries. Until then, this simple approach
wins.

We log to stderr only.

Log level is set at compile time:
- ASPHR_LOGLEVEL_NONE: log nothing
- ASPHR_LOGLEVEL_ERR: log only errors
- ASPHR_LOGLEVEL_WARN: log errors and warnings
- ASPHR_LOGLEVEL_INFO: log errors and warnings and info
- ASPHR_LOGLEVEL_DBG: log everything

Default is ASPHR_LOGLEVEL_DBG.

*/

// ASPHR_LOG_ERR(msg, key, value, key, value, ...)
// ASPHR_LOG_WARN(msg, key, value, key, value, ...)
// ASPHR_LOG_INFO(msg, key, value, key, value, ...)
// ASPHR_LOG_DBG(msg, key, value, key, value, ...)

#if !defined(ASPHR_LOGLEVEL_NONE) && !defined(ASPHR_LOGLEVEL_ERR) &&  \
    !defined(ASPHR_LOGLEVEL_WARN) && !defined(ASPHR_LOGLEVEL_INFO) && \
    !defined(ASPHR_LOGLEVEL_DBG)
#define ASPHR_LOGLEVEL_DBG
#endif

#define ASPHR_EXPAND_LABEL(x) absl::StrCat(" ", #x, "=")
#define ASPHR_EXPAND_VALUE(x) x
#define ASPHR_DO_LOG_INTERNAL_DO_NOT_USE(msg, level, ...)                  \
  {                                                                        \
    absl::Time t1 = absl::Now();                                           \
    auto s = absl::StrCat(                                                 \
        "[",                                                               \
        absl::FormatTime("%Y-%m-%d%ET%H:%M:%E2S%Ez", t1,                   \
                         absl::LocalTimeZone()),                           \
        " ", __FILE__, ":", __LINE__, " ", level, "] ", msg,               \
        __VA_OPT__(ASPHR_FOR_EACH2(ASPHR_EXPAND_LABEL, ASPHR_EXPAND_VALUE, \
                                   __VA_ARGS__), ) "\n");                  \
    std::cerr << s;                                                        \
    std::cerr.flush();                                                     \
  }

#if defined(ASPHR_LOGLEVEL_ERR) || defined(ASPHR_LOGLEVEL_WARN) || \
    defined(ASPHR_LOGLEVEL_INFO) || defined(ASPHR_LOGLEVEL_DBG)
#define ASPHR_LOG_ERR(msg, ...) \
  ASPHR_DO_LOG_INTERNAL_DO_NOT_USE(msg, "ERR", __VA_ARGS__)
#else
#define ASPHR_LOG_ERR(msg, ...) static_cast<void>(0)
#endif

#if defined(ASPHR_LOGLEVEL_WARN) || defined(ASPHR_LOGLEVEL_INFO) || \
    defined(ASPHR_LOGLEVEL_DBG)
#define ASPHR_LOG_WARN(msg, ...) \
  ASPHR_DO_LOG_INTERNAL_DO_NOT_USE(msg, "WARN", __VA_ARGS__)
#else
#define ASPHR_LOG_WARN(msg, ...) static_cast<void>(0)
#endif

#if defined(ASPHR_LOGLEVEL_INFO) || defined(ASPHR_LOGLEVEL_DBG)
#define ASPHR_LOG_INFO(msg, ...) \
  ASPHR_DO_LOG_INTERNAL_DO_NOT_USE(msg, "INFO", __VA_ARGS__)
#else
#define ASPHR_LOG_INFO(msg, ...) static_cast<void>(0)
#endif

#if defined(ASPHR_LOGLEVEL_DBG)
#define ASPHR_LOG_DBG(msg, ...) \
  ASPHR_DO_LOG_INTERNAL_DO_NOT_USE(msg, "DBG", __VA_ARGS__)
#else
#define ASPHR_LOG_DBG(msg, ...) static_cast<void>(0)
#endif
