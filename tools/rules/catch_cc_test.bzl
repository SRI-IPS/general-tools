# To use this rule, add the following to a BUILD file:
#   load("//tools/rules:catch_cc_test.bzl", "catch_cc_test")
def catch_cc_test(
        name,
        srcs = [],
        deps = [],
        **kwargs):
    """A macro that simplifies cc_test rules by adding catch dependencies and a catch_main.
    """
    deps = deps + ["//third_party/Catch:catch"]
    srcs = srcs + ["//third_party/Catch:unittests_main"]
    native.cc_test(
        name = name,
        srcs = srcs,
        deps = deps,
        **kwargs)
