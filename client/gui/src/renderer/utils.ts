//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

import * as React from "react";
import { matchSorter } from "match-sorter";

export function truncate(str: string, maxLength: number) {
  if (str.length <= maxLength) {
    return str;
  }
  return str.substr(0, maxLength - 3) + "...";
}

export function formatTime(date: Date) {
  return date.toLocaleString();
}

export function randomString(length: number) {
  for (
    var s = "";
    s.length < length;
    s +=
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789".charAt(
        (Math.random() * 62) | 0
      )
  );
  return s;
}

export function useSearch<T>(
  searchList: T[],
  search: string,
  searchKeys: string[]
) {
  return React.useMemo(
    () =>
      matchSorter(searchList, search, {
        keys: searchKeys,
      }),
    [searchList, search]
  ) as T[];
}

export function useFocus(): [React.MutableRefObject<any>, () => void] {
  const htmlElRef = React.useRef(null);
  const setFocus = () => {
    if (htmlElRef.current) {
      (htmlElRef.current as any).focus();
    }
  };

  return [htmlElRef, setFocus];
}

export function usePointerMovedSinceMount() {
  const [moved, setMoved] = React.useState(false);

  React.useEffect(() => {
    function handler() {
      setMoved(true);
    }

    if (!moved) {
      window.addEventListener("pointermove", handler);
      return () => window.removeEventListener("pointermove", handler);
    } else {
      return () => {};
    }
  }, [moved]);

  return moved;
}

export function classNames(...classes: string[]) {
  return classes.filter(Boolean).join(" ");
}
