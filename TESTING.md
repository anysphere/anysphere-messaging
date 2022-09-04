## Testing

```bash
bazelisk test //...
```

Testing with address sanitizer is also recommended:

```bash
bazelisk test --config=asan //...
```

For non-Linux machines, you probably want to (1) not build the gRPC schema for JavaScript, and (2) not build libpqxx. The following should work:

```bash
baselizk test //integration_tests/... --define memdb=true
```

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

# One-time setup

Create a GH access token with the full repo scope. Put it in `.env` as `GH_TOKEN=ghp_xxxxxxxx`. Then run `make publish-mac`.

To debug, check `~/Library/Caches/co.anysphere.Anysphere.ShipIt`.
