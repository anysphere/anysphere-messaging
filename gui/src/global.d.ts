//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { Daemon } from "./main/daemon";

declare global {
  // eslint-disable-next-line no-var
  var logger: Console;

  interface Window extends Daemon {
    copyToClipboard(s: string): void;
    isPlatformMac(): boolean;
  }
}
export {};
