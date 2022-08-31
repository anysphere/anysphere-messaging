load("@rules_cc//cc:defs.bzl", "cc_test")

exports_files(["attacker_template.hpp"])

# attack tests
# this first test only needs to be run occasionally.
cc_test(
    name = "attack_template_test",
    srcs = [
        "attacker_template_test.cc",
        "attacker_template_test.hpp",
    ],
    linkstatic = True,
    deps = [
        "//integration_tests:attack_setup",
    ],
)

cc_test(
    name = "pir_replay_test",
    srcs = [
        "pir_replay_test.cc",
        "pir_replay_test.hpp",
    ],
    linkstatic = True,
    deps = [
        "//integration_tests:attack_setup",
    ],
)
