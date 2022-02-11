//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { AfterPackContext, BuildResult } from "electron-builder";
const builder = require("electron-builder");
import path from "path";
import fs from "fs";
import { notarize } from "electron-notarize";

// environment variable options:
// - MAC_ARCH: "universal", "x64", "arm64" or undefined for the current architecture
// - MAC_DONT_NOTARIZE: true to skip notarization
require("dotenv").config({ path: path.resolve(__dirname, "../../.env") });

function base_dir(rel: string) {
  return path.join(path.resolve(__dirname, "../../"), rel);
}

function assets_dir(rel: string) {
  return base_dir(path.join("assets", rel));
}

function release_dir(rel: string) {
  return base_dir(path.join("release", rel));
}

function binaries_dir(rel: string) {
  return base_dir(path.join("binaries", rel));
}

function mac_arch() {
  return process.env.MAC_ARCH;
}

const config = {
  appId: "co.anysphere.Anysphere",
  copyright: "Anysphere",
  productName: "Anysphere",
  asar: true,
  asarUnpack: "**\\*.{node,dll}",

  directories: {
    app: release_dir("app"),
    buildResources: base_dir("assets"),
    output: release_dir("build"),
  },

  // these files are relative to the release_dir("app") directory
  files: ["dist", "package.json", "node_modules"],

  // Make sure that all files declared in "extraResources" exists and abort if they don't.
  afterPack: (context: AfterPackContext) => {
    const resources =
      context.packager.platformSpecificBuildOptions.extraResources;
    for (const resource of resources) {
      if (!fs.existsSync(resource.from)) {
        throw new Error(`Can't find file: ${resource.from}`);
      }
    }
  },

  mac: {
    target: {
      target: "pkg",
      arch: mac_arch(),
    },
    category: "public.app-category.productivity",
    hardenedRuntime: true,
    icon: assets_dir("icon.icns"),
    entitlements: assets_dir("entitlements.mac.plist"),
    gatekeeperAssess: false,
    extraResources: [
      {
        from: binaries_dir("anysphered"), // the daemon
        to: ".",
      },
      {
        from: binaries_dir("anysphere"), // the cli
        to: ".",
      },
      // TODO: add shell completions and an uninstall script here
      // TODO: assets directory needs to contain plist???????
    ],
  },

  pkg: {
    scripts: assets_dir("pkg-scripts"),
    allowAnywhere: false,
    allowCurrentUserHome: false,
    isRelocatable: false,
    isVersionChecked: false,
  },
};

function package_mac() {
  const app_out_dirs: string[] = [];

  return builder.build({
    targets: builder.Platform.MAC.createTarget(),
    config: {
      ...config,
      afterPack: (context: AfterPackContext) => {
        config.afterPack?.(context);

        app_out_dirs.push(context.appOutDir);
        return Promise.resolve();
      },
      afterAllArtifactBuild: async (buildResult: BuildResult) => {
        if (process.env.MAC_DONT_NOTARIZE !== "true") {
          await notarize_mac(buildResult.artifactPaths[0]);
        } else {
          console.log("WARNING: Skipping notarization. Don't publish this!!");
        }
        return [];
      },
      afterSign: async (context: AfterPackContext) => {
        const out_dir = context.appOutDir;
        app_out_dirs.push(out_dir);

        if (process.env.MAC_DONT_NOTARIZE !== "true") {
          const appName = context.packager.appInfo.productFilename;
          await notarize_mac(path.join(out_dir, `${appName}.app`));
        }
      },
    },
  });
}

function notarize_mac(app_path: string) {
  console.log("Notarizing " + app_path);
  return notarize({
    tool: "notarytool",
    appPath: app_path,
    keychain: `${process.env.HOME}/Library/Keychains/login.keychain-db`,
    keychainProfile: "anysphere-gui-profile",
  });
}

package_mac();
