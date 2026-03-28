load("@rules_cc//cc:defs.bzl", "cc_binary")

def add_sample(name):
    cc_binary(
        name = name,
        srcs = ["{}.cpp".format(name)],
        deps = ["//:argparse"],
    )
