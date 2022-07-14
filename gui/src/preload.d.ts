//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { Daemon } from "./main/daemon";

declare global {
  interface Window {
    copyToClipboard(s: string): void;
    isPlatformMac(): boolean;

    daemon: Daemon;
  }
}

export {};
