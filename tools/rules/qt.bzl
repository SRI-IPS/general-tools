# TODO(kgreenek): Make this a real rule instead of just a macro.
def qt_cc_library(name, srcs=[], hdrs=[], moc_hdrs=[], ui_srcs=[], copts=[], deps=[], **kwargs):
    """Compiles a QT library.

    Args:
      name: A name for the target.
      srcs: Normal cc sources.
      hdrs: Normal cc headers.
      moc_hdrs: Headers for classes tagged as QObjects. MOC files will be generated for these files.
      ui_srcs: .ui files. These will be compiled with uic.
      copts: copts that are forwarded to the cc_library rule.
      deps: cc_library dependencies for the library.
    """
    for moc_hdr in moc_hdrs:
        moc_name = moc_hdr.split(".")[0]
        moc_target = "__%s_moc" % moc_name
        native.genrule(
            name = moc_target,
            srcs = [moc_hdr],
            outs = ["moc_%s.cpp" % moc_name],
            cmd =  "moc $(location %s) -o $@ -f'%s'" \
                % (moc_hdr, '%s/%s' % (native.package_name(), moc_hdr)),
        )
        srcs = srcs + [":%s" % moc_target]

    for ui_src in ui_srcs:
        ui_name = ui_src.split(".")[0]
        ui_target = "__%s_ui" % ui_name
        native.genrule(
            name = ui_target,
            srcs = [ui_src],
            outs = ["ui_%s.h" % ui_name],
            cmd = "uic $(locations %s) -o $@" % ui_src,
        )
        srcs = srcs + [":%s" % ui_target]

    native.cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs + moc_hdrs,
        copts = ["-fpic"] + copts,
        deps = deps,
        **kwargs
    )
