load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

# load("@bazel_tools//tools/build_defs/repo:local.bzl", "local_repository")
load("@bazel_tools//tools/cpp:cc_configure.bzl", "cc_configure")

cc_configure()

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

http_archive(
    name = "com_google_googletest",
    sha256 = "5cf189eb6847b4f8fc603a3ffff3b0771c08eec7dd4bd961bfd45477dd13eb73",
    strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
    urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
)

http_archive(
    name = "rules_cc",
    sha256 = "4dccbfd22c0def164c8f47458bd50e0c7148f3d92002cdb459c2a96a68498241",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.1/rules_cc-0.0.1.tar.gz"],
)

# js grpc

http_archive(
    name = "rules_proto_grpc",
    sha256 = "507e38c8d95c7efa4f3b1c0595a8e8f139c885cb41a76cab7e20e4e67ae87731",
    strip_prefix = "rules_proto_grpc-4.1.1",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/4.1.1.tar.gz"],
)

# clang tidy

git_repository(
    name = "bazel_clang_tidy",
    commit = "69aa13e6d7cf102df70921c66be15d4592251e56",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

# libpqxx for postgres
# http_archive(
#     name = "rules_foreign_cc",
#     sha256 = "08b2d21ea7a52fbc2faf202dba8e260b503785975903f183c56e7118876fdf62",
#     strip_prefix = "rules_foreign_cc-605a80355dae9e1855634b733b98a5d97a92f385",
#     url = "https://github.com/bazelbuild/rules_foreign_cc/archive/605a80355dae9e1855634b733b98a5d97a92f385.tar.gz",  # v 0.7.0
# )
http_archive(
    name = "rules_foreign_cc",
    sha256 = "62e364a05370059f07313ec46ae4205af23deb00e41f786f3233a98098c7e636",
    strip_prefix = "rules_foreign_cc-ae4ff42901354e2da8285dac4be8329eea2ea96a",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/ae4ff42901354e2da8285dac4be8329eea2ea96a.tar.gz",  # v 0.7.1
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

# we don't register built tools here because make_tool currently fails to build on Mac
# see this issue: https://github.com/bazelbuild/rules_foreign_cc/issues/859
rules_foreign_cc_dependencies(register_built_tools = False)

new_git_repository(
    name = "libpqxx",
    build_file = "//:libpqxx.BUILD",
    commit = "9100bef4b7488d05d414bd5f58a6811f4dae636e",
    remote = "https://github.com/jtv/libpqxx",
)

# SEAL

new_local_repository(
    name = "seal",
    build_file = "//:seal.BUILD",
    path = "third_party/SEAL",
)

# libsodium

http_archive(
    name = "libsodium",
    build_file = "//:libsodium.BUILD",
    sha256 = "ac87ce227a42405601d4b03e43c3fa61c5ddbf481842af8f0d04fc3d3ff4c968",  # generated using shasum -a 256 <file>.tar.gz
    strip_prefix = "libsodium-7d67f1909bfa6e1225469dbcdb0229c5a9bbf8e2",  # stable branch as of 2022-01-06
    urls = ["https://github.com/jedisct1/libsodium/archive/7d67f1909bfa6e1225469dbcdb0229c5a9bbf8e2.tar.gz"],
)

# grpc and rules_proto

http_archive(
    name = "rules_proto",
    sha256 = "66bfdf8782796239d3875d37e7de19b1d94301e8972b3cbd2446b332429b4df1",
    strip_prefix = "rules_proto-4.0.0",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

# swap proto later with

# git_repository(
#   name = "org_pubref_rules_protobuf",
#   remote = "https://github.com/pubref/rules_protobuf",
#   tag = "v0.8.2",
#   #commit = "..." # alternatively, use latest commit on master
# )
# load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cpp_proto_repositories")
# cpp_proto_repositories()

http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "9647220c699cea4dafa92ec0917c25c7812be51a18143af047e20f3fb05adddc",
    strip_prefix = "grpc-1.43.0",
    urls = ["https://github.com/grpc/grpc/archive/refs/tags/v1.43.0.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()

load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")

rules_proto_grpc_toolchains()

load("@rules_proto_grpc//js:repositories.bzl", rules_proto_grpc_js_repos = "js_repos")

rules_proto_grpc_js_repos()

load("@build_bazel_rules_nodejs//:index.bzl", "yarn_install")

yarn_install(
    name = "npm",
    package_json = "//schema:package.json",
    yarn_lock = "//schema:yarn.lock",
)

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.tar.gz"],
)
