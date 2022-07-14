//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { AfterPackContext, BuildResult, Arch } from "electron-builder";
const builder = require("electron-builder");
import path from "path";
import fs from "fs";
import { notarize } from "electron-notarize";
import { promisify } from "util";
import { exec as execNonPromisified } from "child_process";
import { exit } from "process";
const exec = promisify(execNonPromisified);

// environment variable options:
// - MAC_ARCH: "universal", "x64", "arm64", "both" or undefined for the current architecture
// - MAC_DONT_NOTARIZE: true to skip notarization
require("dotenv").config({ path: path.resolve(__dirname, "../../.env") });

function baseDir(rel: string): string {
  return path.join(path.resolve(__dirname, "../../"), rel);
}

function assetsDir(rel: string): string {
  return baseDir(path.join("assets", rel));
}

function releaseDir(rel: string): string {
  return baseDir(path.join("release", rel));
}

// function binaries_dir(rel: string) {
//   return base_dir(path.join("binaries", rel));
// }

// function binaries_dir_with_arch(rel: string) {
//   return base_dir(path.join("binaries", "${arch}", rel));
// }

function macArch(): string {
  return process.env["MAC_ARCH"] ?? "arm64";
}

const config = {
  appId: "co.anysphere.Anysphere",
  copyright: "Anysphere",
  productName: "Anysphere",
  asar: true,
  asarUnpack: "**\\*.{node,dll}",

  directories: {
    app: releaseDir("app"),
    buildResources: baseDir("assets"),
    output: releaseDir("build"),
  },

  publish: ["github"],

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
      target: "default",
      arch: macArch() === "both" ? ["x64", "arm64"] : macArch(),
    },
    category: "public.app-category.productivity",
    hardenedRuntime: true,
    icon: assetsDir("icon.icns"),
    entitlements: assetsDir("entitlements.mac.plist"),
    gatekeeperAssess: false,
    extraResources: [
      {
        from: path.join(baseDir("binaries")), // daemon and CLI
        to: ".",
        filter: ["${arch}"],
      },
      // TODO: add shell completions and an uninstall script here
    ],
  },

  dmg: {
    sign: false,
  },
};

function packageMac() {
  if (process.env["MAC_DONT_NOTARIZE"] === "true") {
    console.log("WARNING! Skipping notarization");
  }

  const doPublish =
    process.env["PUBLISH_ASPHR"] === "true" ? "always" : "never";

  process.stdout.write(`DO PUBLISH: ${doPublish}\n`);

  return builder.build({
    targets: builder.Platform.MAC.createTarget(),
    publish: doPublish,
    config: {
      ...config,
      afterPack: async (context: AfterPackContext) => {
        config.afterPack?.(context);

        const r = await exec(
          `mv ${context.appOutDir}/Anysphere.app/Contents/Resources/${
            Arch[context.arch]
          } ${context.appOutDir}/Anysphere.app/Contents/Resources/bin`
        );

        if (r.stderr) {
          console.log("ERROR");
          console.log(r.stderr);
          exit(1);
        }

        // for updates, need binaries to be writable
        const r2 = await exec(
          `chmod u+w ${context.appOutDir}/Anysphere.app/Contents/Resources/bin/*`
        );
        if (r2.stderr) {
          console.log("ERROR");
          console.log(r2.stderr);
          exit(1);
        }
      },
      afterAllArtifactBuild: async (buildResult: BuildResult) => {
        console.log(`NOT notarizing: ${buildResult.artifactPaths[0]}`);
        // don't notarize anything here, I believe...
        // if (process.env.MAC_DONT_NOTARIZE !== "true") {
        //   await notarize_mac(buildResult.artifactPaths[0]);
        // } else {
        //   console.log("WARNING: Skipping notarization. Don't publish this!!");
        // }
        return [];
      },
      afterSign: async (context: AfterPackContext) => {
        const out_dir = context.appOutDir;

        if (process.env["MAC_DONT_NOTARIZE"] !== "true") {
          const appName = context.packager.appInfo.productFilename;
          await notarizeMac(path.join(out_dir, `${appName}.app`));
        }
      },
    },
  });
}

function notarizeMac(app_path: string) {
  console.log("Notarizing " + app_path);
  return notarize({
    tool: "notarytool",
    appPath: app_path,
    keychain: `${process.env["HOME"]}/Library/Keychains/login.keychain-db`,
    keychainProfile: "anysphere-gui-profile",
  });
}

packageMac();
