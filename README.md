# Anysphere Client

This repository, along with [anysphere/asphr](https://github.com/anysphere/asphr), contains all client-side code run by Anysphere. You can download Anysphere [here](https://anysphere.co/download), or build your own identical Anysphere client from source by following the instructions below. If you have any questions, please reach out to us at [anysphere.id/#85dG6DNNv...](https://anysphere.id/#85dG6DNNvNQeCNSMXyJ5j3YprmRkmQDe7QcbEuHgoyw81UBLAAAA3NfN3fJgkNUMtrt8vxQBx3wfLn1cg3MJUac99XRwN9rVbn) or `hello@anysphere.co`.

## Build from source

### Warm-up: building the daemon

The daemon runs in the background and interacts with the Anysphere server. For more details on the high-level architecture, read section 5 of [our whitepaper](https://anysphere.co/anysphere-whitepaper.pdf).

1. [Install Bazel using Bazelisk](https://bazel.build/install/bazelisk).
2. Clone [anysphere/asphr](https://github.com/anysphere/asphr) and [anysphere/client](https://github.com/anysphere/client) and put them in a directory side-by-side.
3. In the `client` folder, run `bazelisk build //...`.
4. Run `./bazel-bin/daemon/main` to run the daemon!

It should print something like the following if successful:

```
[2022-08-09T09:21:31.08-04:00 daemon/main.cc:83 INFO] Parsed args. db_address=/Users/arvid/.anysphere/daemon/asphr.db socket_address=unix:///Users/arvid/.anysphere/run/anysphere.sock
[2022-08-09T09:21:31.09-04:00 daemon/main.cc:93 INFO] Querying server. server_address=server1.anysphere.co:443
```

### Building the entire client

This builds the full app, which is what you can also download from [anysphere.co/download](https://anysphere.co/download).

1. Go to the `gui` directory: `cd gui`.
2. Run `npm run package-mac`.

Linux and Windows support will be added soon.

## Developing

### Useful commands

1. `bazelisk build //...` to build everything.
2. `bazelisk test //...` to test everything.

## Detailed building instructions

Run

```bash
npm run package-mac
```

Make sure you have Xcode installed.

To enable notarization, first run

```bash
security find-identity -p basic -v
```

to see the developer certificates (there should be Developer ID Application and Developer ID Installer). Make note of the ten character code at the end: this is the `teamId`. Then run

```bash
xcrun notarytool store-credentials --apple-id "name@example.com" --team-id "ABCD123456" --keychain ~/Library/Keychains/login.keychain-db
```

with your Apple ID and `teamId`. Enter profile name `anysphere-gui-profile` and an app-specific password generated on https://appleid.apple.com/.

Then everything should just work! For more details, read https://scriptingosx.com/2021/07/notarize-a-command-line-tool-with-notarytool/.

Let `.env` contain the environment variables specified by `helpers/scripts/package-mac.ts`.

## Manual testing

Build:

```bash
bazelisk build //...
```

In one terminal, run a normal daemon:

```bash
./bazel-bin/daemon/main
```

In a new terminal, run a second daemon:

```bash
./wrap2.sh ./bazel-bin/daemon/main
```

To connect to daemon 1, run:

```bash
./bazel-bin/cli/asphr [command]
```

To connect to daemon 2, run:

```bash
./wrap2.sh ./bazel-bin/cli/asphr [command]
```

For the gui, run:

```bash
./wrap2.sh ./gui/start.sh
```

## Installing the CLI

Run:

```bash
sudo mkdir -p /usr/local/bin
sudo ln -sf /Applications/Anysphere.app/Contents/Resources/bin/anysphere /usr/local/bin/anysphere
cat << EOF >> ~/.zprofile
export PATH="\$PATH:/usr/local/bin"
EOF
```

Replace `.zprofile` with `.bash_profile` if you use bash instead.
