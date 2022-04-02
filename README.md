# Anysphere Client

This repository, along with [anysphere/asphr](https://github.com/anysphere/asphr), contains all client-side code run by Anysphere! Feel free to poke around. If you have any questions or concerns, please email us at `founders@anysphere.co`.

## Build

Clone anysphere/asphr and anysphere/client and put them in a directory side-by-side. Then, in the `client` directory, run:

```
bazel build //...
```

to build, and

```
bazel test //...
```

to test.

## Develop GUI.

Checkout [the GUI Readme](gui/README.md)!

## Package

```
npm run package-mac
```

Make sure you have Xcode installed.

To enable notarization, first run

```
security find-identity -p basic -v
```

to see the developer certificates (there should be Developer ID Application and Developer ID Installer). Make note of the ten character code at the end: this is the teamId. Then run

```
xcrun notarytool store-credentials --apple-id "name@example.com" --team-id "ABCD123456" --keychain ~/Library/Keychains/login.keychain-db
```

with your Apple ID and teamId. Enter profile name `anysphere-gui-profile` and an app-specific password generate on https://appleid.apple.com/.

Then everything should just work! For more details, read https://scriptingosx.com/2021/07/notarize-a-command-line-tool-with-notarytool/.

Let `.env` contain the environment variables specified by `helpers/scripts/package-mac.ts`.

## Manual test

Build:

```
bazel build //...
```

In one terminal, run normal daemon:

```
./bazel-bin/daemon/daemon
```

In a new terminal, run a second daemon:

```
./wrap2.sh ./bazel-bin/daemon/daemon
```

To connect to daemon 1, run:

```
./bazel-bin/cli/asphr [command]
```

To connect to daemon 2, run:

```
./wrap2.sh ./bazel-bin/cli/asphr [command]
```
