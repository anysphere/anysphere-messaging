/* eslint-disable @typescript-eslint/naming-convention */
//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

/** @type {import('tailwindcss').Config} */
module.exports = {
  purge: ["./src/**/*.{js,jsx,ts,tsx}"],
  plugins: [require("@tailwindcss/forms"), require("@tailwindcss/typography")],
  theme: {
    extend: {
      typography: ({ theme }) => ({
        stonex: {
          css: {
            // "--tw-prose-bullets": theme("colors.asbrown['dark']"),
            "--tw-prose-bullets": theme("colors.asbrown[light]"),
          },
        },
      }),
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
        asbeige2: "#FCFBF8",
        asyellow: {
          light: "#FECA6F",
          500: "#E2A924",
          dark: "#CB8E00",
        },
      },
      animation: {
        wiggle: "wiggle 1s ease-in-out",
        "fade-in-down": "fade-in-down 0.5s ease-out",
        reveal: "fade-in-down 0.7s ease-out",
        revealsimple: "fade-in 1.5s ease-out",
        sendmsg: "sendmsg 10s linear infinite",
        retrievemsg: "retrievemsg 10s linear infinite",
        slideunderline: "slideunderline 1s ease-out",
        blob: "blob 7s infinite",
      },
      keyframes: {
        wiggle: {
          "0%, 100%": { transform: "rotate(-3deg)" },
          "50%": { transform: "rotate(3deg)" },
        },
        "fade-in": {
          "0%": {
            opacity: "0",
          },
          "50%": {
            opacity: "0",
          },
          "100%": {
            opacity: "0.8",
          },
        },
        "fade-in-down": {
          "0%": {
            opacity: "0",
            transform: "translateY(-10px)",
          },
          "100%": {
            opacity: "1",
            transform: "translateY(0)",
          },
        },
        slideunderline: {
          "0%, 30%": {
            textDecorationColor: "rgba(0,0,0,0)",
            textUnderlineOffset: "12px",
          },
          "100%": {
            textDecorationColor: "rgba(0,0,0,1)",
            textUnderlineOffset: "4px",
          },
        },
        sendmsg: {
          "0%": {},
          "10%": {
            transform: "translate(5em, -5.3rem)",
          },
          "50%": {
            transform: "translate(5em, -5.3rem)",
          },
          "63%, 70%": {
            transform: "translate(5em, 0)",
          },
          "87%, 100%": {
            transform: "translate(11em, 0)",
          },
        },
        retrievemsg: {
          "0%": {},
          "25%": {
            transform: "translate(0, 0)",
          },
          "35%": {
            transform: "translate(-5em, -6.35em)",
          },
          "46%": {
            transform: "translate(-5em, -6.35em)",
          },
          "50%": {
            transform: "translate(-5em, -5.3em)",
          },
          "63%, 70%": {
            transform: "translate(-5em, 0)",
          },
          "87%, 100%": {
            transform: "translate(0.5em, 0)",
          },
        },
        blob: {
          "0%": {
            transform: "translate(0px, 0px) scale(1)",
          },
          "33%": {
            transform: "translate(30px, -50px) scale(1.1)",
          },
          "66%": {
            transform: "translate(-20px, 20px) scale(0.9)",
          },
          "100%": {
            transform: "tranlate(0px, 0px) scale(1)",
          },
        },
      },
    },
  },
};
