def all_tests_in_folder(folder, name = ""):
    for t in native.glob([folder + "/*"]):
        native.cc_test(
            name = name + t[:-3],
            srcs = [
                t,
            ],
            linkstatic=True,
            deps = [
                ":daemon_setup",
            ],
        )
