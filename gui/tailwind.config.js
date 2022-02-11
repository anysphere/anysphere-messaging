//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

module.exports = {
  purge: ["./src/**/*.{js,jsx,ts,tsx}"],
  theme: {
    extend: {
      colors: {
        asbrown: {
          100: "#E4DFD2",
          200: "#ADA798",
          300: "#827D70",
          light: "#575245",
          dark: "#252116",
        },
        asgreen: {
          light: "#287A57",
          dark: "#194F39",
        },
        asbeige: "#F9F7F1",
        asyellow: {
          light: "#FECA6F",
          500: "#E2A924",
          dark: "#CB8E00",
        },
      },
    },
  },
  plugins: [],
};
