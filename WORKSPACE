#
# Copyright 2022 Anysphere, Inc.
#

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "asphr",
    commit = "882f70540e467400f8d00bf7df2c023e47087cbe",  # autoupdate anysphere/asphr
    init_submodules = True,
    remote = "https://github.com/anysphere/asphr.git",
)
# for local builds, we add --override_repository=asphr=asphr, which we do in setupgit.sh

load("@asphr//:asphr_load.bzl", "load_asphr_repos")

load_asphr_repos("@asphr")

load("@asphr//:asphr_load2.bzl", "load_asphr_repos2")

load_asphr_repos2()

load("@asphr//:asphr.bzl", "setup_asphr")

setup_asphr("@asphr")

git_repository(
    name = "client",
    commit = "4346f5f630d9b27ecb686a609c83042ad9844860",  # autoupdate anysphere/client
    init_submodules = True,
    remote = "https://github.com/anysphere/client.git",
)
# for local builds, we add --override_repository=client=client, which we do in setupgit.sh

# docker stuff
http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "59536e6ae64359b716ba9c46c39183403b01eabfbd57578e84398b4829ca499a",
    strip_prefix = "rules_docker-0.22.0",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.22.0/rules_docker-v0.22.0.tar.gz"],
)

load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)

container_repositories()

load(
    "@io_bazel_rules_docker//cc:image.bzl",
    _cc_image_repos = "repositories",
)

_cc_image_repos()

git_repository(
    name = "bazel_clang_tidy",
    commit = "69aa13e6d7cf102df70921c66be15d4592251e56",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

new_git_repository(
    name = "libpqxx",
    build_file = "//:libpqxx.BUILD",
    commit = "9100bef4b7488d05d414bd5f58a6811f4dae636e",
    remote = "https://github.com/jtv/libpqxx",
)
