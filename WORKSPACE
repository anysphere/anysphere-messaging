load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/cpp:cc_configure.bzl", "cc_configure")

cc_configure()

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
    sha256 = "5cf189eb6847b4f8fc603a3ffff3b0771c08eec7dd4bd961bfd45477dd13eb73"
    urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
)

# rules_cc for clang_tidy
http_archive(
    name = "rules_cc",
    sha256 = "3cde212ccda3ba152897e7fd354c42eba275878b6d98fe4f2125c684a73f3842",
    strip_prefix = "rules_cc-d66a13e2a01630afcafc4ba411d83e291ecf02bd",
    urls = [
        "https://github.com/bazelbuild/rules_cc/archive/d66a13e2a01630afcafc4ba411d83e291ecf02bd.tar.gz",
    ],
)

git_repository(
    name = "bazel_clang_tidy",
    commit = "69aa13e6d7cf102df70921c66be15d4592251e56",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

# libpqxx for postgres
http_archive(
    name = "rules_foreign_cc",
    sha256 = "5470b22db8974824a1994593f117b0ef8c2e5fb474fd30a7473fcfd77d8d1ed7",
    strip_prefix = "rules_foreign_cc-f01fd353ee2adcd55aab899c12fa2733223228a1",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/f01fd353ee2adcd55aab899c12fa2733223228a1.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

new_git_repository(
    name = "libpqxx",
    build_file = "//:libpqxx.BUILD",
    commit = "9100bef4b7488d05d414bd5f58a6811f4dae636e",
    remote = "https://github.com/jtv/libpqxx",
)

# SEAL

new_git_repository(
    name = "seal",
    build_file = "//:seal.BUILD",
    commit = "4ee204b3a2d472160045109ac8fffd25e81004ba",  # branch: 3.7.2-anysphere
    remote = "https://github.com/anysphere/SEAL",
    # tag = "v3.7.2-bazel",
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
    strip_prefix = "grpc-master",
    sha256 = "977f09ad233b8f6674ba3bf366254f18db01d4b95c636386db19019e614fbe1b"
    urls = ["https://github.com/grpc/grpc/archive/master.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()

new_git_repository(
    name = "com_github_daniele77_cli",
    build_file = "//:cli_header_lib.BUILD",
    remote = "https://github.com/daniele77/cli.git",
    tag = "v2.0.0",
)
