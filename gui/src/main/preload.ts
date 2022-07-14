//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { contextBridge, clipboard } from "electron";
import { DaemonImpl } from "./daemon";

contextBridge.exposeInMainWorld("copyToClipboard", (s: string) => {
  clipboard.writeText(s, "selection");
});

contextBridge.exposeInMainWorld("isPlatformMac", () => {
  return process.platform === "darwin";
});

const daemonI = new DaemonImpl();
const classToObject = (theClass) => {
  const originalClass = theClass || {};
  const keys = Object.getOwnPropertyNames(Object.getPrototypeOf(originalClass));
  return keys.reduce((classAsObj, key) => {
    classAsObj[key] = originalClass[key];
    return classAsObj;
  }, {});
};
contextBridge.exposeInMainWorld("daemon", classToObject(daemonI));
