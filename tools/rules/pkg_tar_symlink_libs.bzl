load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")

# This rule is meant for non-bazel dependencies that install shared libs. Sometimes they install
# versioned shared libs, where the versioned so files are symlinks. E.g. (libfoo.so -> libfoo.so.1).
# If you use the standard pkg_tar rule, it will break the symlinks, resulting in full copies of the
# shared libs. This can significantly increase the size of the resulting tar file.
# This rule is a wrapper around pkg_tar that detects when shared libs have been copied in this way,
# and re-creates them as symlinks.
def pkg_tar_symlink_libs(name, extension="tar", **kwargs):
    pkg_tar_target = "__%s_pkg_tar" % name
    # NOTE: This intentionally does not pass the extension. The compression for tar.gz files is
    # handled by the genrule below.
    pkg_tar(
        name = pkg_tar_target,
        **kwargs
    )
    out_path = "%s.%s" % (name, extension)
    native.genrule(
        name = name,
        srcs = [pkg_tar_target],
        outs = [out_path],
        tools = ["//tools:tar_symlink_libs.py"],
        output_to_bindir = True,
        cmd = "python3 $(location //tools:tar_symlink_libs.py) --in_tar $< --out_tar $@"
    )
