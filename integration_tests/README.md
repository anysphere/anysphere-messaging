# How to check which tests are in the folder

```
bazel query 'tests(//integration_tests/...)'
```

# How to run individual tests

From the root of the project:

```
bazelisk test //integration_tests:{folder_name}/{test_name}
```

From the integration test folder

```
bazelisk test :{folder_name}/{test_name}
```
