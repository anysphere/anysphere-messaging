# BUILD

```
bazel build //src:anysphere
```

# RUN

```
bazel run //src:anysphere
```

# TEST

```
bazel test --test_output=all //test:as_test
```

# LOCAL BUILD

```
bazel build //client/... --host_javabase=@local_jdk//:jdk
```

Note that the schema and the server do not really want to build on Mac M1. This is fine. We only need to build the client locally.

# LOCAL RUN

```
bazel run //client/daemon --host_javabase=@local_jdk//:jdk --action_env="HOME=$HOME"
```

# PACKAGE

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
