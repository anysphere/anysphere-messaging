# BUILD

```
bazel build //src:as_server
```

# RUN
```
bazel run //src:as_server
```

# TEST

```
bazel test --test_output=all //test:server_test --host_javabase=@local_jdk//:jdk
```


## using postgres