//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "as_cli.hpp"

auto make_anysphere_menu() -> unique_ptr<cli::Menu> {
  auto rootMenu = make_unique<cli::Menu>("anysphere");

  rootMenu->Insert(
      "menu", [](std::ostream& out) { out << "Hello, world\n"; },
      "The Anysphere menu");
  rootMenu->Insert(
      "color",
      [](std::ostream& out) {
        out << "Colors ON\n";
        cli::SetColor();
      },

      "Enable colors in the cli");
  rootMenu->Insert(
      "nocolor",
      [](std::ostream& out) {
        out << "Colors OFF\n";
        cli::SetNoColor();
      },
      "Disable colors in the cli");
  return rootMenu;
}
