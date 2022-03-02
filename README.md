# anysphere

The world's first completely private messenger. For freedom.

Clone: `git clone git@github.com:anysphere/anysphere && cd anysphere && ./setupgit.sh`

# Git

I recommend using the following git aliases:
1. `git ll` for recursive pulling
2. `git pp` for recursive pushing
3. `git ss` for recursive status
4. `git cc` for recursive committing
5. `git ca` for recursive committing all

(look in the `githooks` folder for an exhaustive list of these)

To get these working, you need to run `./setupgit.sh` and follow its instructions.

# Testing

```
bazel test //...
```

Testing with address sanitizer is also recommended:

```
bazel test --config=asan //...
```

# GitPod

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/anysphere/anysphere)

# Build for x86

Install `x86brew`:

```
arch --x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

Create alias: add `alias x86brew='arch --x86_64 /usr/local/Homebrew/bin/brew'` to your `.bashrc`.
Install `x86bazel`:

```
x86brew install bazel
```

Create alias: add `alias x86bazel='arch --x86_64 /usr/local/bin/bazel'` to your `.bashrc`.
Make sure `which bazel` is the `/opt/homebrew/...` version, and make sure `x86bazel` is the right thing too.
