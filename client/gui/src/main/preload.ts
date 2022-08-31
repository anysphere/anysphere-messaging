//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { contextBridge, clipboard } from "electron";
import { DaemonImpl } from "./daemon";

contextBridge.exposeInMainWorld("copyToClipboard", (s: string) => {
  clipboard.writeText(s);
});

contextBridge.exposeInMainWorld("isPlatformMac", () => {
  return process.platform === "darwin";
});

const daemonI = new DaemonImpl();
const methods = Object.getOwnPropertyNames(
  Object.getPrototypeOf(daemonI)
).filter((x) => x !== "constructor");
for (const method of methods) {
  contextBridge.exposeInMainWorld(method, (...args) => {
    return daemonI[method](...args);
  });
}
