# anysphere

The world's first completely private messenger. For freedom.

Clone: `git clone git@github.com:anysphere/anysphere && cd anysphere && ./setupgit.sh`

## Git

I recommend using the following git aliases:

1. `git ll` for recursive pulling
2. `git pp` for recursive pushing
3. `git ss` for recursive status
4. `git cc` for recursive committing
5. `git ca` for recursive committing all

(look in the `githooks` folder for an exhaustive list of these)

To get these working, you need to run `./setupgit.sh` and follow its instructions.

## Updating schema

After updating the schema in `asphr/schema`, run the following on a Linux machine:

```bash
make update-gui-repo
```

in this directory. Then commit and push! This updates the generated schema for the gui.

## Testing

```bash
bazel test //...
```

Testing with address sanitizer is also recommended:

```bash
bazel test --config=asan //...
```

## GitPod

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/anysphere/anysphere)

## Build for x86

Install `x86brew`:

```bash
arch --x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

Create alias: add `alias x86brew='arch --x86_64 /usr/local/Homebrew/bin/brew'` to your `.bashrc`.
Install `x86bazel`:

```bash
x86brew install bazel
```

Create alias: add `alias x86bazel='arch --x86_64 /usr/local/bin/bazel'` to your `.bashrc`.
Make sure `which bazel` is the `/opt/homebrew/...` version, and make sure `x86bazel` is the right thing too.

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

### One-time setup

Create a GH access token with the full repo scope. Put it in `.env` as `GH_TOKEN=ghp_xxxxxxxx`. Then run `make publish-mac`.

To debug, check `~/Library/Caches/co.anysphere.Anysphere.ShipIt`.

# Profiling

First the simple usage to profile a binary:

### Simple Usage

1. `bazelisk build //... --config perf`
2. `perf record ${binary} // The binary can be found in the bazel-bin directory`
3. `perf report`

### Perf errors about not having enough access to CPU counter

```
# set "kernel.perf_event_paranoid = 0" in "/etc/sysctl.conf" which allows profiler to access symbols.
echo "Setting kernel.perf_event_paranoid = 0"
sudo echo "kernel.perf_event_paranoid = 0" >> /etc/sysctl.conf # you might have to do this manually.

# Restart the service.
sudo sysctl --system &> /dev/null
```
