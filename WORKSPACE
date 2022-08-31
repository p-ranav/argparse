workspace(name = "com_github_p_ranav_argparse")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_cc",
    sha256 = "4aeb102efbcfad509857d7cb9c5456731e8ce566bfbf2960286a2ec236796cc3",
    strip_prefix = "rules_cc-2f8c04c04462ab83c545ab14c0da68c3b4c96191",
    url = "https://github.com/bazelbuild/rules_cc/archive/2f8c04c04462ab83c545ab14c0da68c3b4c96191.tar.gz",
)

load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies", "rules_cc_toolchains")

rules_cc_dependencies()

rules_cc_toolchains()
