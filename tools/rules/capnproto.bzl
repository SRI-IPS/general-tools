def _capnp_gen_impl(ctx):
    includes = depset(ctx.attr.includes + [ctx.attr.capnp_capnp.label.workspace_root + "/c++/src"])
    inputs = depset(ctx.files.srcs)
    for dep_target in ctx.attr.deps:
        includes += dep_target.capnp.includes
        inputs += dep_target.capnp.inputs

    cc_out = "-o%s:%s" % (ctx.executable.capnpc_cxx.path, ctx.genfiles_dir.path)
    args = ["compile", "--verbose", "--no-standard-import", cc_out]
    include_flags = ["-I" + inc for inc in includes.to_list()]

    ctx.actions.run(
        inputs=list(inputs.to_list()) + ctx.files.capnpc_cxx + ctx.files.capnp_capnp,
        outputs=ctx.outputs.outs,
        executable=ctx.executable.capnp,
        arguments=args + include_flags + [s.path for s in ctx.files.srcs],
        mnemonic="GenCapnp",
        tools=[],
    )

    return struct(
        capnp=struct(
            includes = includes,
            inputs = inputs,
        )
    )

_capnp_gen = rule(
    implementation=_capnp_gen_impl,
    attrs={
        "srcs": attr.label_list(allow_files=True),
        "deps": attr.label_list(providers=["capnp"]),
        "includes": attr.string_list(),
        "capnp": attr.label(
            executable=True,
            cfg="host",
            allow_single_file=True,
            mandatory=True,
        ),
        "capnpc_cxx": attr.label(
            executable=True,
            cfg="host",
            allow_single_file=True,
            mandatory=True,
        ),
        "capnp_capnp": attr.label(mandatory=True),
        "outs": attr.output_list(),
    },
    output_to_genfiles=True,
)

def cc_capnp_library(
        name,
        srcs=[],
        deps=[],
        includes=[],
        capnproto="@com_github_sandstorm_io_capnproto//:capnproto",
        capnp="@com_github_sandstorm_io_capnproto//:capnp",
        capnpc_cxx="@com_github_sandstorm_io_capnproto//:capnpc-c++",
        capnp_capnp="@com_github_sandstorm_io_capnproto//:capnp-capnp",
        **kargs):
    """Bazel rule to create a C++ capnproto library from capnp source files
    """
    outs = [s + ".h" for s in srcs] + [s + ".c++" for s in srcs]
    _capnp_gen(
        name=name + "_gencapnp",
        srcs=srcs,
        deps=[s + "_gencapnp" for s in deps],
        includes=includes,
        capnp=capnp,
        capnpc_cxx=capnpc_cxx,
        capnp_capnp=capnp_capnp,
        outs=outs,
        visibility=["//visibility:public"],
    )
    native.cc_library(
        name=name,
        srcs=outs,
        deps=[capnproto] + deps,
        includes=includes,
        **kargs)
