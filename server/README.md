# BUILD

```
bazelisk build //server/src:as_server
```

# RUN

```
bazelisk run //server/src:as_server
```

# TEST

```
bazelisk test --test_output=all //server/test:server_test --host_javabase=@local_jdk//:jdk
```

## manual testing

use `grpcurl` to test the server. it is great.

## debugging

compile with debug symbols

```
bazelisk build //server/test:server_test --compilation_mode=dbg
```

then run gdb:

```
gdb --args bazel-bin/server/test/server_test --gtest_break_on_failure --gtest_catch_exceptions=0
```

## using in memory db

```
bazelisk run //server/src:as_server --define memdb=true
```

to use a non-persisting in-memory db instead of postgres

## using postgres

start a postgres container:

`make run` in `database`.

Test: `bazelisk test //... --define postgres_tests=true`
