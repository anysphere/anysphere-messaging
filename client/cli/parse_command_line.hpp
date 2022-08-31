//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "asphr/asphr.hpp"

struct CommandLine {
  int argc;
  char** argv;
  // vector<string> argList;
  string comLine;

  CommandLine(int _c, char** _v, std::string _cl)
      : argc(_c), argv(_v), comLine(std::move(_cl)) {
    // argList = vector<string>(argv, argv + argc);
  }

  CommandLine(int _c, char** _v)
      : argc(_c), argv(_v), comLine("bad arguments") {
    // TODO(Sualeh): be nice and use a vector :)
    // argList = vector<string>(argv, argv + argc);
  }

  auto badArgument() const {
    auto err = comLine;
    return absl::InvalidArgumentError(err);
  }

  // get an argument
  auto getArgument(int i) -> asphr::StatusOr<string> {
    if (argc < i + 1) {
      return absl::InvalidArgumentError("please see the help message :)");
    }

    return (string)argv[i];
  }

  auto getOption(const std::string& option) -> bool {
    for (int i = 1; i < argc; i++)
      if ((string)argv[i] == option) return true;
    return false;
  }

  auto getOptionValue(const std::string& option, char* defaultValue) {
    for (int i = 1; i < argc - 1; i++)
      if ((string)argv[i] == option) return argv[i + 1];
    return defaultValue;
  }

  auto getOptionValue(const string& option, const string& defaultValue)
      -> asphr::StatusOr<string> {
    for (int i = 1; i < argc - 1; i++)
      if ((string)argv[i] == option) return string(argv[i + 1]);
    return absl::InvalidArgumentError("option " + option +
                                      " not found in command line");
  }

  auto getConcatArguments(int start) -> asphr::StatusOr<string> {
    string result;
    for (int i = start; i < argc; i++) result = StrCat(result, " ", argv[i]);
    return result;
  }
};