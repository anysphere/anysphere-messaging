import { ArchType } from "builder-util";
import { Configuration } from "electron-builder";
import { AfterPackContext } from "electron-builder";
import path from "path";
import fs from "fs";

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

export const config: Configuration = {
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
    // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
    const resources =
      // eslint-disable-next-line @typescript-eslint/no-unsafe-member-access
      context.packager.platformSpecificBuildOptions.extraResources;
    for (const resource of resources) {
      // eslint-disable-next-line @typescript-eslint/strict-boolean-expressions, @typescript-eslint/no-unsafe-member-access, @typescript-eslint/no-unsafe-call, @typescript-eslint/no-unsafe-argument
      if (!fs.existsSync(resource.from)) {
        // eslint-disable-next-line @typescript-eslint/no-unsafe-member-access
        throw new Error(`Can't find file: ${resource.from}`);
      }
    }
  },

  protocols: [
    {
      name: "Anysphere URL",
      schemes: ["anysphere"],
      role: "Editor",
    },
  ],

  mac: {
    target: {
      target: "default",
      arch: macArch() === "both" ? ["x64", "arm64"] : (macArch() as ArchType),
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

  linux: {
    target: {
      target: "AppImage",
      arch: "x64", // TODO: support arm64
    },
    category: "public.app-category.productivity",
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
