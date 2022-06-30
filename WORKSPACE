#
# Copyright 2022 Anysphere, Inc.
#

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "asphr",
    commit = "c7fcff2549426bb3bd4198faabd9e15592f1ec3b",  # autoupdate anysphere/asphr
    init_submodules = True,
    remote = "https://github.com/anysphere/asphr.git",
)
# for local builds, we add --override_repository=asphr=asphr, which we do in setupgit.sh

load("@asphr//:asphr_load.bzl", "load_asphr_repos")

load_asphr_repos("@asphr")

load("@asphr//:asphr_load2.bzl", "load_asphr_repos2")

load_asphr_repos2()

load("@asphr//:asphr_load3.bzl", "load_asphr_repos3")

load_asphr_repos3()

load("@asphr//:asphr_load4.bzl", "load_asphr_repos4")

load_asphr_repos4()

load("@asphr//:asphr_load5.bzl", "load_asphr_repos5")

load_asphr_repos5("@asphr")

load("@asphr//:asphr.bzl", "setup_asphr")

setup_asphr("@asphr")

git_repository(
    name = "client",
    commit = "d4f4a42398894b7945503c01dce3f32b292650bf",  # autoupdate anysphere/client
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
