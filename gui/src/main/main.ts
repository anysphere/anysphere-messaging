//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

/* eslint global-require: off, no-console: off, promise/always-return: off */

import path from "path";
import { app, BrowserWindow, session, shell, Notification } from "electron";
import MenuBuilder from "./menu";
import { resolveHtmlPath } from "./util";

import {
  getDaemonClient,
  convertProtobufIncomingMessageToTypedMessage,
  truncate,
} from "./daemon";
import daemonM from "../daemon/schema/daemon_pb";

if (process.env.NODE_ENV === "production") {
  const sourceMapSupport = require("source-map-support");
  sourceMapSupport.install();
}

const isDevelopment =
  process.env.NODE_ENV === "development" || process.env.DEBUG_PROD === "true";

const installExtensions = async () => {
  const installer = require("electron-devtools-installer");
  const forceDownload = !!process.env.UPGRADE_EXTENSIONS;
  const extensions = ["REACT_DEVELOPER_TOOLS"];

  return installer
    .default(
      extensions.map((name) => installer[name]),
      forceDownload
    )
    .catch(console.log);
};

const createWindow = async () => {
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

  mainWindow.loadURL(resolveHtmlPath("index.html"));

  mainWindow.on("ready-to-show", () => {
    if (!mainWindow) {
      throw new Error('"mainWindow" is not defined');
    }
    if (process.env.START_MINIMIZED) {
      mainWindow.minimize();
    } else {
      mainWindow.show();
    }
  });

  const menuBuilder = new MenuBuilder(mainWindow);
  menuBuilder.buildMenu();

  // Don't allow ANY requests to any origin! This means that the app will
  // only not be able to communicate with the internet at all, which is PERFECT.
  session.defaultSession.webRequest.onHeadersReceived((details, callback) => {
    callback({
      responseHeaders: {
        ...details.responseHeaders,
        "Content-Security-Policy": [
          "default-src 'self' style-src 'self' 'unsafe-inline'",
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

function registerForNotifications() {
  const daemonClient = getDaemonClient();
  const request = new daemonM.GetMessagesRequest();
  request.setFilter(daemonM.GetMessagesRequest.Filter.NEW);
  var call = daemonClient.getMessagesStreamed(request);

  let firstTime = true;

  call.on("data", function (r) {
    if (firstTime) {
      firstTime = false;
      return;
    }
    try {
      const lm = r.getMessagesList();
      const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
      // notify!!!
      for (const m of l) {
        const notification = new Notification({
          title: m.from,
          body: truncate(m.message, 50),
        });
        notification.show();
      }
    } catch (e) {
      console.log(`error in getNewMessagesStreamed: ${e}`);
    }
    console.log("got all messages streamed", r);
  });
  call.on("end", function () {
    // The server has finished sending
    // TODO(arvid): resubscribe?
    console.log("getNewMessagesStreamed end");
  });
  call.on("error", function (e) {
    // An error has occurred and the stream has been closed.
    // TODO(arvid): resubscribe?
    console.log("getNewMessagesStreamed error", e);
  });
  call.on("status", function (status) {
    // process status
    console.log("getNewMessagesStreamed status", status);
  });
  return () => {
    console.log("cancelling notifications!");
    call.cancel();
  };
}

app
  .whenReady()
  .then(() => {
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
