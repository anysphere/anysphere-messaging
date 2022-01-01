# BUILD

```
bazel build //server/src:as_server
```

# RUN
```
bazel run //server/src:as_server
```

# TEST

```
bazel test --test_output=all //server/test:server_test --host_javabase=@local_jdk//:jdk
```


## using in memory db

```
bazel run //server/src:as_server --define memdb=true
```

to use a non-persisting in-memory db instead of postgres

## using postgres

start a postgres container:

`make run` in `database`.

TODO: transition this make into bazel but bazel is hard.