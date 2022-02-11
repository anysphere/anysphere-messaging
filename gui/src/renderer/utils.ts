//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

export function truncate(str: string, maxLength: number) {
  if (str.length <= maxLength) {
    return str;
  }
  return str.substr(0, maxLength - 3) + "...";
}

export function formatTime(date: Date) {
  return date.toLocaleString();
}
