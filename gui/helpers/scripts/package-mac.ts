import { AfterPackContext } from "electron-builder";
import path from "path";
import fs from "fs";

// environment variable options:
// - MAC_UNIVERSAL: true for universal, false for build for current architecture only

function assets(rel: string) {
  return path.join(path.resolve(__dirname, "./assets"), rel);
}

function release_dir(rel: string) {
  return path.join(path.resolve(__dirname, "./release"), rel);
}

function binaries(rel: string) {
  return path.join(path.resolve(__dirname, "./binaries"), rel);
}

function mac_arch() {
  if (process.env.MAC_UNIVERSAL) {
    return "universal";
  } else {
    return undefined; // build for current architecture
  }
}

const config = {
  appId: "co.anysphere.Anysphere",
  copyright: "Anysphere",
  productName: "Anysphere",
  asar: true,
  asarUnpack: "**\\*.{node,dll}",
  extraResources: [assets("entitlements.mac.plist")],

  directories: {
    app: release_dir("app"),
    buildResources: assets("."),
    output: release_dir("build"),
  },

  // these files are relative to the release_dir("app") directory
  files: ["dist", "package.json", "node_modules"],

  // Make sure that all files declared in "extraResources" exists and abort if they don't.
  afterPack: (context: AfterPackContext) => {
    const resources =
      context.packager.platformSpecificBuildOptions.extraResources;
    for (const resource of resources) {
      if (!fs.existsSync(resource)) {
        throw new Error(`Can't find file: ${resource}`);
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
    icon: assets("icon.icns"),
    entitlements: assets("entitlements.mac.plist"),
    gatekeeperAssess: false,
    extraResources: [
      {
        from: binaries("anysphered"), // the daemon
        to: ".",
      },
      {
        from: binaries("anysphere"), // the cli
        to: ".",
      },
      // TODO: add shell completions and an uninstall script here
      // TODO: binaries directory needs to contain plist???????
      // TODO: maybe just put the binaries in the assets folder????? seems reasonable????
    ],
  },

  pkg: {
    allowAnywhere: false,
    allowCurrentUserHome: false,
    isRelocatable: false,
    isVersionChecked: false,
  },

  nsis: {
    guid: "2A356FD4-03B7-4F45-99B4-737BE580DC82",
    oneClick: false,
    perMachine: true,
    allowElevation: true,
    allowToChangeInstallationDirectory: false,
    include: distAssets("windows/installer.nsh"),
    installerSidebar: distAssets("windows/installersidebar.bmp"),
  },

  win: {
    target: [
      {
        target: "nsis",
        arch: ["x64"],
      },
    ],
    artifactName: "MullvadVPN-${version}.${ext}",
    publisherName: "Mullvad VPN AB",
    signingHashAlgorithms: ["sha256"],
    signDlls: true,
    extraResources: [
      { from: distAssets("mullvad.exe"), to: "." },
      { from: distAssets("mullvad-problem-report.exe"), to: "." },
      { from: distAssets("mullvad-daemon.exe"), to: "." },
      { from: distAssets("talpid_openvpn_plugin.dll"), to: "." },
      {
        from: root(
          path.join(
            "windows",
            "winfw",
            "bin",
            "x64-${env.CPP_BUILD_MODE}",
            "winfw.dll"
          )
        ),
        to: ".",
      },
      {
        from: root(
          path.join(
            "windows",
            "windns",
            "bin",
            "x64-${env.CPP_BUILD_MODE}",
            "windns.dll"
          )
        ),
        to: ".",
      },
      {
        from: root(
          path.join(
            "windows",
            "winnet",
            "bin",
            "x64-${env.CPP_BUILD_MODE}",
            "winnet.dll"
          )
        ),
        to: ".",
      },
      {
        from: distAssets("binaries/x86_64-pc-windows-msvc/openvpn.exe"),
        to: ".",
      },
      {
        from: distAssets("binaries/x86_64-pc-windows-msvc/sslocal.exe"),
        to: ".",
      },
      { from: root("build/lib/x86_64-pc-windows-msvc/libwg.dll"), to: "." },
      {
        from: distAssets("binaries/x86_64-pc-windows-msvc/wintun/wintun.dll"),
        to: ".",
      },
      {
        from: distAssets(
          "binaries/x86_64-pc-windows-msvc/wireguard-nt/mullvad-wireguard.dll"
        ),
        to: ".",
      },
    ],
  },

  linux: {
    target: ["deb", "rpm"],
    artifactName: "MullvadVPN-${version}_${arch}.${ext}",
    category: "Network",
    icon: distAssets("icon.icns"),
    extraFiles: [
      { from: distAssets("linux/mullvad-gui-launcher.sh"), to: "." },
    ],
    extraResources: [
      { from: distAssets("mullvad-problem-report"), to: "." },
      { from: distAssets("mullvad-daemon"), to: "." },
      { from: distAssets("mullvad-setup"), to: "." },
      { from: distAssets("libtalpid_openvpn_plugin.so"), to: "." },
      {
        from: distAssets("binaries/x86_64-unknown-linux-gnu/openvpn"),
        to: ".",
      },
      {
        from: distAssets("binaries/x86_64-unknown-linux-gnu/sslocal"),
        to: ".",
      },
      { from: distAssets("linux/mullvad-daemon.conf"), to: "." },
      { from: distAssets("linux/mullvad-daemon.service"), to: "." },
    ],
  },

  deb: {
    fpm: [
      "--no-depends",
      "--version",
      getDebVersion(),
      "--before-install",
      distAssets("linux/before-install.sh"),
      "--before-remove",
      distAssets("linux/before-remove.sh"),
      "--config-files",
      "/opt/Mullvad VPN/resources/mullvad-daemon.service",
      "--config-files",
      "/opt/Mullvad VPN/resources/mullvad-daemon.conf",
      distAssets("mullvad") + "=/usr/bin/",
      distAssets("mullvad-exclude") + "=/usr/bin/",
      distAssets("linux/problem-report-link") +
        "=/usr/bin/mullvad-problem-report",
      distAssets("shell-completions/mullvad.bash") +
        "=/usr/share/bash-completion/completions/mullvad",
      distAssets("shell-completions/_mullvad") +
        "=/usr/local/share/zsh/site-functions/_mullvad",
      distAssets("shell-completions/mullvad.fish") +
        "=/usr/share/fish/vendor_completions.d/mullvad.fish",
    ],
    afterInstall: distAssets("linux/after-install.sh"),
    afterRemove: distAssets("linux/after-remove.sh"),
  },

  rpm: {
    fpm: [
      "--before-install",
      distAssets("linux/before-install.sh"),
      "--before-remove",
      distAssets("linux/before-remove.sh"),
      "--rpm-posttrans",
      distAssets("linux/post-transaction.sh"),
      "--config-files",
      "/opt/Mullvad VPN/resources/mullvad-daemon.service",
      "--config-files",
      "/opt/Mullvad VPN/resources/mullvad-daemon.conf",
      distAssets("mullvad") + "=/usr/bin/",
      distAssets("mullvad-exclude") + "=/usr/bin/",
      distAssets("linux/problem-report-link") +
        "=/usr/bin/mullvad-problem-report",
      distAssets("shell-completions/mullvad.bash") +
        "=/usr/share/bash-completion/completions/mullvad",
      distAssets("shell-completions/_mullvad") +
        "=/usr/share/zsh/site-functions/_mullvad",
      distAssets("shell-completions/mullvad.fish") +
        "=/usr/share/fish/vendor_completions.d/mullvad.fish",
    ],
    afterInstall: distAssets("linux/after-install.sh"),
    afterRemove: distAssets("linux/after-remove.sh"),
    depends: ["libXScrnSaver", "libnotify", "libnsl", "dbus-libs"],
  },
};

const config2 = {
  productName: "Anysphere",
  appId: "co.anysphere.Anysphere",
  asar: true,
  asarUnpack: "**\\*.{node,dll}",
  files: ["dist", "node_modules", "package.json"],
  afterSign: "helpers/scripts/notarize.js",
  mac: {
    target: {
      target: "default",
      arch: ["arm64", "x64"],
    },
    type: "distribution",
    hardenedRuntime: true,
    entitlements: "assets/entitlements.mac.plist",
    entitlementsInherit: "assets/entitlements.mac.plist",
    gatekeeperAssess: false,
  },
  dmg: {
    contents: [
      {
        x: 130,
        y: 220,
      },
      {
        x: 410,
        y: 220,
        type: "link",
        path: "/Applications",
      },
    ],
  },
  win: {
    target: ["nsis"],
  },
  linux: {
    target: ["AppImage"],
    category: "Development",
  },
  directories: {
    app: "release/app",
    buildResources: "assets",
    output: "release/build",
  },
  extraResources: ["./assets/**"],
};
