//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import path from "path";
import rimraf from "rimraf";
import webpackPaths from "../configs/webpack.paths";

export default function deleteSourceMaps() {
  rimraf.sync(path.join(webpackPaths.distMainPath, "*.js.map"));
  rimraf.sync(path.join(webpackPaths.distRendererPath, "*.js.map"));
}
