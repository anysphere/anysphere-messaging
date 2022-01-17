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
