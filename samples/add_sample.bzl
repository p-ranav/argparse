def add_sample(name):
    native.cc_binary(
        name = name,
        srcs = ["{}.cpp".format(name)],
        deps = ["//:argparse"],
    )
