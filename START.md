# Getting Started

Clone the repo, including all submodules:

```
git clone --recursive git@github.com:anysphere/anysphere-messaging
```

## GitPod

We support and use gitpod often. You can start here if you would like to try it out.

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/anysphere/anysphere)

## Updating schema

After updating the schema in `asphr/schema`, run the following on a Linux machine:

```bash
make update-gui-repo
```

in this directory. Then commit and push! This updates the generated schema for the gui.

## Build for x86

Install `x86brew`:

```bash
arch --x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

Create alias: add `alias x86brew='arch --x86_64 /usr/local/Homebrew/bin/brew'` to your `.bashrc`.
Install `x86bazelisk`:

```bash
x86brew install bazelisk
```

Create alias: add `alias x86bazelisk='arch --x86_64 /usr/local/bin/bazelisk` to your `.bashrc`.
Make sure `which bazelisk` is the `/opt/homebrew/...` version, and make sure `x86bazelisk` is the right thing too.

## Release

Make a new draft release on GitHub in the client repo. Pick a new version number and give the tag `vX.X.X`.

Run

```bash
ASPHR_VERSION=X.X.X make publish-mac
```

Publish the release on GitHub.

Deploy the landing page:

```bash
make publish-landing
```
