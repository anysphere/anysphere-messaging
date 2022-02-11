//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import fs from "fs";
import webpackPaths from "../configs/webpack.paths";

const srcNodeModulesPath = webpackPaths.srcNodeModulesPath;
const appNodeModulesPath = webpackPaths.appNodeModulesPath;

if (!fs.existsSync(srcNodeModulesPath) && fs.existsSync(appNodeModulesPath)) {
  fs.symlinkSync(appNodeModulesPath, srcNodeModulesPath, "junction");
}
