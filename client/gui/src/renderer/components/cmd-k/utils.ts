//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
// import type { Action } from "./types";

// https://stackoverflow.com/questions/13382516/getting-scroll-bar-width-using-javascript
export function getScrollbarWidth() {
  const outer = document.createElement("div");
  outer.style.visibility = "hidden";
  outer.style.overflow = "scroll";
  document.body.appendChild(outer);
  const inner = document.createElement("div");
  outer.appendChild(inner);
  const scrollbarWidth = outer.offsetWidth - inner.offsetWidth;
  outer.parentNode!.removeChild(outer);
  return scrollbarWidth;
}

export function shouldRejectKeystrokes(
  {
    ignoreWhenFocused,
  }: {
    ignoreWhenFocused: string[];
  } = { ignoreWhenFocused: [] }
) {
  const inputs = ["input", "textarea", ...ignoreWhenFocused].map((el) =>
    el.toLowerCase()
  );

  const activeElement = document.activeElement;
  const ignoreStrokes =
    activeElement &&
    (inputs.indexOf(activeElement.tagName.toLowerCase()) !== -1 ||
      activeElement.attributes.getNamedItem("role")?.value === "textbox" ||
      activeElement.attributes.getNamedItem("contenteditable")?.value ===
        "true");

  return ignoreStrokes;
}

const SSR = typeof window === "undefined";
const isMac = !SSR && window.isPlatformMac();
export function isModKey(
  event: KeyboardEvent | MouseEvent | React.KeyboardEvent
) {
  return isMac ? event.metaKey : event.ctrlKey;
}

export function useThrottledValue(value: any, ms: number = 100) {
  const [throttledValue, setThrottledValue] = React.useState(value);
  const lastRan = React.useRef(Date.now());

  React.useEffect(() => {
    const timeout = setTimeout(() => {
      setThrottledValue(value);
      lastRan.current = Date.now();
    }, lastRan.current - (Date.now() - ms));

    return () => {
      clearTimeout(timeout);
    };
  }, [ms, value]);

  return throttledValue;
}
