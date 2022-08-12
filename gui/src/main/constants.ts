import { systemPreferences } from "electron";
import { exit } from "process";
import path from "path";

// this commit hash will be automatically updated by gui/package.json.
export const RELEASE_COMMIT_HASH = "bf027e552c8103a83e8c1f206c4acbbd616ec120";

export const PLIST_PATH = (): string => {
  if (process.platform === "darwin" && process.env["HOME"] != null) {
    return path.join(
      process.env["HOME"],
      "Library",
      "LaunchAgents",
      "co.anysphere.anysphered.plist"
    );
  } else {
    process.stderr.write("Platform not supported");
    exit(1);
  }
};

export const PLIST_CONTENTS = (pkgPath: string, logPath: string): string => {
  return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>co.anysphere.anysphered</string>
    <key>Program</key>
    <string>${pkgPath}/bin/anysphered</string>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>${logPath}/anysphered.log</string>
    <key>StandardErrorPath</key>
    <string>${logPath}/anysphered.err</string>
</dict>
</plist>`;
};

export const SYSTEMD_UNIT_CONTENTS = (
  pkgPath: string,
  logPath: string
): string => {
  return `[Unit]
Description=Anysphere daemon.
After=network.target

[Service]
Type=simple
WorkingDirectory=~
ExecStart=${pkgPath}/bin/anysphered
StandardOutput=append:${logPath}/anysphered.log
StandardError=append:${logPath}/anysphered.err
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target`;
};
