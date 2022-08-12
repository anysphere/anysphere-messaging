//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import path from "path";
import {
  app,
  dialog,
  BrowserWindow,
  session,
  shell,
  Notification,
} from "electron";
import MenuBuilder from "./menu";
import { resolveHtmlPath } from "./util";
import { autoUpdater } from "electron-updater";
import { promisify } from "util";
import { exec as execNonPromisified } from "child_process";
const exec = promisify(execNonPromisified);

import { truncate, DaemonImpl, getConfigDir } from "./daemon";
import { IncomingMessage } from "../types";
import * as daemon_pb from "../daemon/schema/daemon_pb";
import {
  PLIST_CONTENTS,
  PLIST_PATH,
  RELEASE_COMMIT_HASH,
  SYSTEMD_UNIT_CONTENTS,
} from "./constants";
import { exit } from "process";
import fs from "fs";
import { Console } from "console";

const daemon = new DaemonImpl();

const isDevelopment =
  process.env["NODE_ENV"] === "development" ||
  process.env["DEBUG_PROD"] === "true";

if (process.env["NODE_ENV"] === "production") {
  const sourceMapSupport = require("source-map-support");
  sourceMapSupport.install();
}

if (process.defaultApp) {
  if (process.argv.length >= 2 && process.argv[1] != null) {
    app.setAsDefaultProtocolClient("anysphere", process.execPath, [
      path.resolve(process.argv[1]),
    ]);
  }
} else {
  app.setAsDefaultProtocolClient("anysphere");
}

function setupLogger(): void {
  let logPath = "";
  if (process.env["XDG_CACHE_HOME"] !== undefined) {
    logPath = path.join(process.env["XDG_CACHE_HOME"], "anysphere", "logs");
  } else if (process.env["HOME"] !== undefined) {
    logPath = path.join(process.env["HOME"], ".anysphere", "cache", "logs");
  } else {
    process.stderr.write(
      "$HOME or $XDG_CACHE_HOME not set! Cannot create log path, aborting :("
    );
    exit(1);
  }
  app.setAppLogsPath(logPath);

  // in debug mode, use console.log. in production, use the log file.
  if (isDevelopment) {
    global.logger = console;
  } else {
    global.logger = new Console({
      stdout: fs.createWriteStream(path.join(logPath, "anysphere-gui.log")),
      stderr: fs.createWriteStream(path.join(logPath, "anysphere-gui.err")),
    });
  }
}

async function startDaemonIfNeeded(pkgPath: string): Promise<void> {
  try {
    const daemonStatus = await daemon.getStatus({});
    // if release hash is wrong, we need to restart!
    if (daemonStatus.releaseHash !== RELEASE_COMMIT_HASH) {
      throw new Error("incorrect release hash");
    }
    // daemon is running, correct version, nothing to do
    logger.log("Daemon is running, with the correct version!");
    return;
  } catch (e) {
    // if development, we don't want to start the daemon (want to do it manually)
    if (isDevelopment) {
      logger.log(
        `Daemon is either not running or running the wrong version. Please start it; we're not doing anything because we're in DEV mode. Error: ${e}.`
      );
      return;
    } else {
      logger.log(
        `Daemon is either not running or running the wrong version. Error: ${e}.`
      );
    }

    // We do not copy the CLI here, because installing the CLI is a pain.
    // Particularly, we would need to modify the user's PATH.
    // TODO: expose a cmd-K command to install the CLI. It presents the user with a
    // copy-able short command to run in the terminal.
    //
    // first copy the CLI
    // const cliPath = path.join(pkgPath, "bin", "anysphere");
    // // ln -sf link it!
    // // TODO(arvid): just add to PATH instead, because not everyone has /usr/local/bin in their PATH
    // const mkdir = await exec(`mkdir -p /usr/local/bin`);
    // if (mkdir.stderr) {
    //   logger.error(mkdir.stderr);
    // }
    // const clilink = await exec(`ln -sf ${cliPath} /usr/local/bin/anysphere`);
    // if (clilink.stderr) {
    //   logger.error(clilink.stderr);
    // }
    // logger.log("Successfully linked the CLI.");

    // TODO(arvid): handle windows

    if (process.platform === "darwin") {
      // possible problem, so let's start the daemon!
      // unload the plist if it exists.
      const plistPath = PLIST_PATH();
      // 0: create the directory
      const mkdirPlist = await exec(`mkdir -p ${path.dirname(plistPath)}`);
      if (mkdirPlist.stderr) {
        logger.error(mkdirPlist.stderr);
      }
      logger.log("Successfully created the plist directory.");
      // 1: unload plist
      await exec("launchctl unload " + plistPath); // we don't care if it fails or not!
      const logPath = app.getPath("logs");
      const contents = PLIST_CONTENTS(pkgPath, logPath);
      logger.log("Successfully unloaded the plist.");
      // 2: write plist
      await fs.promises.writeFile(plistPath, contents);
      logger.log("Successfully wrote the new plist.");
      // 3: load plist
      const response = await exec("launchctl load " + plistPath);
      if (response.stderr) {
        logger.error(response.stderr);
        exit(1);
      }
      logger.log("Successfully loaded the new plist.");
    } else if (process.platform === "linux") {
      const servicePath = path.join(
        getConfigDir(),
        "co.anysphere.anysphered.service"
      );
      // 0: create the directory
      const mkdir = await exec(`mkdir -p ${path.dirname(servicePath)}`);
      if (mkdir.stderr) {
        logger.error(mkdir.stderr);
      }
      // 1: create the service file
      const logPath = app.getPath("logs");
      const contents = SYSTEMD_UNIT_CONTENTS(pkgPath, logPath);
      await fs.promises.writeFile(servicePath, contents);
      // 2: enable the service in user mode, and run it
      const response = await exec(
        "systemctl enable --now --user " + servicePath
      );
      if (response.stderr) {
        logger.error(response.stderr);
        exit(1);
      }
      logger.log("Successfully started the anysphere daemon.");
    }
  }
}

async function installExtensions(): Promise<void> {
  const installer = require("electron-devtools-installer");
  const forceDownload = !!process.env["UPGRADE_EXTENSIONS"];
  const extensions = ["REACT_DEVELOPER_TOOLS"];

  return installer
    .default(
      extensions.map((name) => installer[name]),
      forceDownload
    )
    .catch(logger.log);
}

const createWindow = async (publicID?: string): Promise<void> => {
  if (isDevelopment) {
    await installExtensions();
  }

  const RESOURCES_PATH = app.isPackaged
    ? path.join(process.resourcesPath, "assets")
    : path.join(__dirname, "../../assets");

  const getAssetPath = (...paths: string[]): string => {
    return path.join(RESOURCES_PATH, ...paths);
  };

  const mainWindow = new BrowserWindow({
    show: false,
    width: 1024,
    minWidth: 200,
    minHeight: 200,
    height: 728,
    backgroundColor: "#F9F7F1",
    icon: getAssetPath("icon.png"),
    titleBarStyle: "hidden",
    webPreferences: {
      preload: app.isPackaged
        ? path.join(__dirname, "preload.js")
        : path.join(__dirname, "../../helpers/dll/preload.js"),
    },
  });

  let loadURL = resolveHtmlPath("index.html");
  if (publicID != null) {
    loadURL = resolveHtmlPath(`index.html?publicID=${publicID}`);
  }

  mainWindow.loadURL(loadURL);

  mainWindow.on("ready-to-show", () => {
    if (process.env["START_MINIMIZED"] === "true") {
      mainWindow.minimize();
    } else {
      mainWindow.show();
    }
  });

  const menuBuilder = new MenuBuilder(mainWindow);
  menuBuilder.buildMenu();

  // Don't allow ANY requests to any origin! This means that the app will
  // only not be able to communicate with the internet at all, which is PERFECT.
  session.defaultSession.enableNetworkEmulation({
    offline: true,
  });
  session.defaultSession.webRequest.onHeadersReceived((details, callback) => {
    callback({
      responseHeaders: {
        ...details.responseHeaders,
        // eslint-disable-next-line @typescript-eslint/naming-convention
        "Content-Security-Policy": [
          "default-src 'self'; style-src 'self' 'unsafe-inline'", // required for tailwind
          // "default-src 'self'",
        ],
      },
    });
  });
};

/**
 * Add event listeners...
 */

app.on("window-all-closed", () => {
  // Respect the OSX convention of having the application in memory even
  // after all windows have been closed
  if (process.platform !== "darwin") {
    app.quit();
  }
});

function registerForNotifications(): () => void {
  let firstTime = true;
  const cancel = daemon.getMessagesStreamed(
    { filter: daemon_pb.GetMessagesRequest.Filter.NEW },
    (messages: IncomingMessage[]) => {
      if (firstTime) {
        firstTime = false;
        return;
      }
      for (const m of messages) {
        const notification = new Notification({
          title: m.fromDisplayName,
          body: truncate(m.message, 50),
        });
        notification.show();
      }
    }
  );

  return () => {
    console.log("cancelling notifications!");
    cancel();
  };
}

app
  .whenReady()
  .then(() => {
    setupLogger();

    autoUpdater.checkForUpdatesAndNotify();

    startDaemonIfNeeded(path.dirname(app.getAppPath()));

    createWindow();
    app.on("activate", () => {
      // On macOS it's common to re-create a window in the app when the
      // dock icon is clicked and there are no other windows open.
      if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
      }
    });

    // set up notifications!
    const cancelNotifications = registerForNotifications();

    app.on("will-quit", () => {
      cancelNotifications();
    });
  })
  .catch(console.log);

app.on("web-contents-created", (event, contents) => {
  contents.on("will-navigate", (event, navigationUrl) => {
    event.preventDefault();
    shell.openExternal(navigationUrl);
  });
});

// Handle the protocol. In this case, we choose to show an Error Box.
app.on("open-url", (event, url) => {
  const publicID = url.split("/").pop();
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow(publicID);
  } else {
    BrowserWindow.getAllWindows()[0].loadURL(
      resolveHtmlPath("index.html?publicID=" + publicID)
    );
  }
});
