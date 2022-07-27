import { systemPreferences } from "electron";
import { exit } from "process";
import path from "path";

// this commit hash will be automatically updated by gui/package.json.
export const RELEASE_COMMIT_HASH = "e1e606952ff47d097718dee42cd922edcf7782f6";

export const PLIST_PATH = () => {
  if (process.platform === "darwin" && process.env.HOME) {
    return path.join(
      process.env.HOME,
      "Library",
      "LaunchAgents",
      "co.anysphere.anysphered.plist"
    );
  } else {
    process.stderr.write("Platform not supported");
    exit(1);
  }
};

export const PLIST_CONTENTS = (pkgPath: string, logPath: string) => {
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
