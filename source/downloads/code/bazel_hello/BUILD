cc_library(
    name = "hello_lib",
    srcs = ["hello.cpp"],
    hdrs = ["hello.h"],
)

cc_binary(
    name = "hello_bin",
    srcs = ["main.cpp"],
    deps = [":hello_lib"],
)

cc_test(
    name = "hello_test",
    srcs = ["hello_test.cpp"],
    deps = [":hello_lib", "@googletest//:gtest_main"],
)
