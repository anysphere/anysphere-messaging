//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { AfterPackContext, BuildResult, Arch } from "electron-builder";
import builder from "electron-builder";
import path from "path";
import { notarize } from "electron-notarize";
import { promisify } from "util";
import { exec as execNonPromisified } from "child_process";
import { exit } from "process";
import { config } from "./package-shared";
const exec = promisify(execNonPromisified);

// environment variable options:
// - LINUX_ARCH: TODO.
require("dotenv").config({ path: path.resolve(__dirname, "../../.env") });

function packageLinux(): Promise<Array<string>> {
  const doPublish =
    process.env["PUBLISH_ASPHR"] === "true" ? "always" : "never";

  process.stdout.write(`DO PUBLISH: ${doPublish}\n`);

  return builder.build({
    targets: builder.Platform.LINUX.createTarget(),
    publish: doPublish,
    // config: {
    //   ...config,
    //   afterPack: async (context: AfterPackContext) => {
    //     config.afterPack?.(context);

    //     // const r = await exec(
    //     //   `mv ${context.appOutDir}/Anysphere.app/Contents/Resources/${
    //     //     Arch[context.arch]
    //     //   } ${context.appOutDir}/Anysphere.app/Contents/Resources/bin`
    //     // );

    //     // if (r.stderr) {
    //     //   console.log("ERROR");
    //     //   console.log(r.stderr);
    //     //   exit(1);
    //     // }

    //     // // for updates, need binaries to be writable
    //     // const r2 = await exec(
    //     //   `chmod u+w ${context.appOutDir}/Anysphere.app/Contents/Resources/bin/*`
    //     // );
    //     // if (r2.stderr) {
    //     //   console.log("ERROR");
    //     //   console.log(r2.stderr);
    //     //   exit(1);
    //     // }
    //   },
    //   afterAllArtifactBuild: async (buildResult: BuildResult) => {
    //     // console.log(`NOT notarizing: ${buildResult.artifactPaths[0]}`);
    //     return [];
    //   },
    //   afterSign: async (context: AfterPackContext) => {
    //     const out_dir = context.appOutDir;

    //     if (process.env["MAC_DONT_NOTARIZE"] !== "true") {
    //       const appName = context.packager.appInfo.productFilename;
    //       await notarizeMac(path.join(out_dir, `${appName}.app`));
    //     }
    //   },
    // },
    config,
  });
}

packageLinux();
