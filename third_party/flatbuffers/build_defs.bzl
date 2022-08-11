# Description:
#   BUILD rules for generating flatbuffer files in various languages.

"""
Rules for building C++ flatbuffers with Bazel.
"""

load("@rules_cc//cc:defs.bzl", "cc_library")

flatc_path = "//third_party/flatbuffers:flatc"

DEFAULT_INCLUDE_PATHS = [
    "./",
    "$(GENDIR)",
    "$(BINDIR)",
]

DEFAULT_FLATC_ARGS = [
    "--gen-object-api",
    "--gen-compare",
    "--no-includes",
    "--gen-mutable",
    "--reflect-names",
    "--cpp-ptr-type flatbuffers::unique_ptr",
]

def flatbuffer_library_public(
        name,
        srcs,
        outs,
        language_flag,
        out_prefix = "",
        includes = [],
        include_paths = DEFAULT_INCLUDE_PATHS,
        flatc_args = DEFAULT_FLATC_ARGS,
        reflection_name = "",
        reflection_visibility = None,
        compatible_with = None,
        restricted_to = None,
        output_to_bindir = False):
    """Generates code files for reading/writing the given flatbuffers in the requested language using the public compiler.

    Args:
      name: Rule name.
      srcs: Source .fbs files. Sent in order to the compiler.
      outs: Output files from flatc.
      language_flag: Target language flag. One of [-c, -j, -js].
      out_prefix: Prepend this path to the front of all generated files except on
          single source targets. Usually is a directory name.
      includes: Optional, list of filegroups of schemas that the srcs depend on.
      include_paths: Optional, list of paths the includes files can be found in.
      flatc_args: Optional, list of additional arguments to pass to flatc.
      reflection_name: Optional, if set this will generate the flatbuffer
        reflection binaries for the schemas.
      reflection_visibility: The visibility of the generated reflection Fileset.
      output_to_bindir: Passed to genrule for output to bin directory.
      compatible_with: Optional, The list of environments this rule can be
        built for, in addition to default-supported environments.
      restricted_to: Optional, The list of environments this rule can be built
        for, instead of default-supported environments.
      output_to_bindir: Passed to genrule for output to bin directory.


    This rule creates a filegroup(name) with all generated source files, and
    optionally a Fileset([reflection_name]) with all generated reflection
    binaries.
    """
    include_paths_cmd = ["-I %s" % (s) for s in include_paths]

    # '$(@D)' when given a single source target will give the appropriate
    # directory. Appending 'out_prefix' is only necessary when given a build
    # target with multiple sources.
    output_directory = (
        ("-o $(@D)/%s" % (out_prefix)) if len(srcs) > 1 else ("-o $(@D)")
    )
    genrule_cmd = " ".join([
        "SRCS=($(SRCS));",
        "for f in $${SRCS[@]:0:%s}; do" % len(srcs),
        "$(location %s)" % (flatc_path),
        " ".join(include_paths_cmd),
        " ".join(flatc_args),
        language_flag,
        output_directory,
        "$$f;",
        "done",
    ])
    native.genrule(
        name = name,
        srcs = srcs + includes,
        outs = outs,
        output_to_bindir = output_to_bindir,
        tools = [flatc_path],
        cmd = genrule_cmd,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        message = "Generating flatbuffer files for %s:" % (name),
    )
    if reflection_name:
        reflection_genrule_cmd = " ".join([
            "SRCS=($(SRCS));",
            "for f in $${SRCS[@]:0:%s}; do" % len(srcs),
            "$(location %s)" % (flatc_path),
            "-b --schema",
            " ".join(flatc_args),
            " ".join(include_paths_cmd),
            language_flag,
            output_directory,
            "$$f;",
            "done",
        ])
        reflection_outs = [
            (out_prefix + "%s.bfbs") % (s.replace(".fbs", "").split("/")[-1])
            for s in srcs
        ]
        native.genrule(
            name = "%s_srcs" % reflection_name,
            srcs = srcs + includes,
            outs = reflection_outs,
            output_to_bindir = output_to_bindir,
            tools = [flatc_path],
            compatible_with = compatible_with,
            restricted_to = restricted_to,
            cmd = reflection_genrule_cmd,
            message = "Generating flatbuffer reflection binary for %s:" % (name),
        )
        native.filegroup(
            name = "%s_out" % reflection_name,
            srcs = reflection_outs,
            visibility = reflection_visibility,
            compatible_with = compatible_with,
            restricted_to = restricted_to,
        )

def flatbuffer_cc_library(
        name,
        srcs,
        srcs_filegroup_name = "",
        out_prefix = "",
        includes = [],
        include_paths = DEFAULT_INCLUDE_PATHS,
        flatc_args = DEFAULT_FLATC_ARGS,
        visibility = None,
        compatible_with = None,
        restricted_to = None,
        srcs_filegroup_visibility = None,
        gen_reflections = False):
    '''A cc_library with the generated reader/writers for the given flatbuffer definitions.

    Args:
      name: Rule name.
      srcs: Source .fbs files. Sent in order to the compiler.
      srcs_filegroup_name: Name of the output filegroup that holds srcs. Pass this
          filegroup into the `includes` parameter of any other
          flatbuffer_cc_library that depends on this one's schemas.
      out_prefix: Prepend this path to the front of all generated files. Usually
          is a directory name.
      includes: Optional, list of filegroups of schemas that the srcs depend on.
          ** SEE REMARKS BELOW **
      include_paths: Optional, list of paths the includes files can be found in.
      flatc_args: Optional list of additional arguments to pass to flatc
          (e.g. --gen-mutable).
      visibility: The visibility of the generated cc_library. By default, use the
          default visibility of the project.
      srcs_filegroup_visibility: The visibility of the generated srcs filegroup.
          By default, use the value of the visibility parameter above.
      gen_reflections: Optional, if true this will generate the flatbuffer
        reflection binaries for the schemas.
      compatible_with: Optional, The list of environments this rule can be built
        for, in addition to default-supported environments.
      restricted_to: Optional, The list of environments this rule can be built
        for, instead of default-supported environments.

    This produces:
      filegroup([name]_srcs): all generated .h files.
      filegroup(srcs_filegroup_name if specified, or [name]_includes if not):
          Other flatbuffer_cc_library's can pass this in for their `includes`
          parameter, if they depend on the schemas in this library.
      Fileset([name]_reflection): (Optional) all generated reflection binaries.
      cc_library([name]): library with sources and flatbuffers deps.

    Remarks:
      ** Because the genrule used to call flatc does not have any trivial way of
        computing the output list of files transitively generated by includes and
        --gen-includes (the default) being defined for flatc, the --gen-includes
        flag will not work as expected. The way around this is to add a dependency
        to the flatbuffer_cc_library defined alongside the flatc included Fileset.
        For example you might define:

        flatbuffer_cc_library(
            name = "my_fbs",
            srcs = [ "schemas/foo.fbs" ],
            includes = [ "//third_party/bazz:bazz_fbs_includes" ],
        )

        In which foo.fbs includes a few files from the Fileset defined at
        //third_party/bazz:bazz_fbs_includes. When compiling the library that
        includes foo_generated.h, and therefore has my_fbs as a dependency, it
        will fail to find any of the bazz *_generated.h files unless you also
        add bazz's flatbuffer_cc_library to your own dependency list, e.g.:

        cc_library(
            name = "my_lib",
            deps = [
                ":my_fbs",
                "//third_party/bazz:bazz_fbs"
            ],
        )

        Happy dependent Flatbuffering!
    '''
    output_headers = [
        (out_prefix + "%s_generated.h") % (s.replace(".fbs", "").split("/")[-1].split(":")[-1])
        for s in srcs
    ]
    reflection_name = "%s_reflection" % name if gen_reflections else ""

    srcs_lib = "%s_srcs" % (name)
    flatbuffer_library_public(
        name = srcs_lib,
        srcs = srcs,
        outs = output_headers,
        language_flag = "-c",
        out_prefix = out_prefix,
        includes = includes,
        include_paths = include_paths,
        flatc_args = flatc_args,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        reflection_name = reflection_name,
        reflection_visibility = visibility,
    )
    cc_library(
        name = name,
        hdrs = [
            ":" + srcs_lib,
        ],
        srcs = [
            ":" + srcs_lib,
        ],
        features = [
            "-parse_headers",
        ],
        deps = [
            "//third_party/flatbuffers:runtime_cc",
        ],
        includes = [],
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        linkstatic = 1,
        visibility = visibility,
    )

    # A filegroup for the `srcs`. That is, all the schema files for this
    # Flatbuffer set.
    native.filegroup(
        name = srcs_filegroup_name if srcs_filegroup_name else "%s_includes" % (name),
        srcs = srcs,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        visibility = srcs_filegroup_visibility if srcs_filegroup_visibility != None else visibility,
    )

# Custom provider to track dependencies transitively.
FlatbufferInfo = provider(
    fields = {
        "transitive_srcs": "flatbuffer schema definitions.",
    },
)

def _flatbuffer_schemas_aspect_impl(target, ctx):
    _ignore = [target]
    transitive_srcs = depset()
    if hasattr(ctx.rule.attr, "deps"):
        for dep in ctx.rule.attr.deps:
            if FlatbufferInfo in dep:
                transitive_srcs = depset(dep[FlatbufferInfo].transitive_srcs, transitive = [transitive_srcs])
    if hasattr(ctx.rule.attr, "srcs"):
        for src in ctx.rule.attr.srcs:
            if FlatbufferInfo in src:
                transitive_srcs = depset(src[FlatbufferInfo].transitive_srcs, transitive = [transitive_srcs])
            for f in src.files:
                if f.extension == "fbs":
                    transitive_srcs = depset([f], transitive = [transitive_srcs])
    return [FlatbufferInfo(transitive_srcs = transitive_srcs)]

# An aspect that runs over all dependencies and transitively collects
# flatbuffer schema files.
_flatbuffer_schemas_aspect = aspect(
    attr_aspects = [
        "deps",
        "srcs",
    ],
    implementation = _flatbuffer_schemas_aspect_impl,
)

# Rule to invoke the flatbuffer compiler.
def _gen_flatbuffer_srcs_impl(ctx):
    outputs = ctx.attr.outputs
    include_paths = ctx.attr.include_paths
    if ctx.attr.no_includes:
        no_includes_statement = ["--no-includes"]
    else:
        no_includes_statement = []

    if ctx.attr.language_flag == "--python":
        onefile_statement = ["--gen-onefile"]
    else:
        onefile_statement = []

    # Need to generate all files in a directory.
    if not outputs:
        outputs = [ctx.actions.declare_directory("{}_all".format(ctx.attr.name))]
        output_directory = outputs[0].path
    else:
        outputs = [ctx.actions.declare_file(output) for output in outputs]
        output_directory = outputs[0].dirname

    deps = depset(ctx.files.srcs + ctx.files.deps, transitive = [
        dep[FlatbufferInfo].transitive_srcs
        for dep in ctx.attr.deps
        if FlatbufferInfo in dep
    ])

    include_paths_cmd_line = []
    for s in include_paths:
        include_paths_cmd_line.extend(["-I", s])

    for src in ctx.files.srcs:
        ctx.actions.run(
            inputs = deps,
            outputs = outputs,
            executable = ctx.executable._flatc,
            arguments = [
                            ctx.attr.language_flag,
                            "-o",
                            output_directory,
                            # Allow for absolute imports and referencing of generated files.
                            "-I",
                            "./",
                            "-I",
                            ctx.genfiles_dir.path,
                            "-I",
                            ctx.bin_dir.path,
                        ] + no_includes_statement +
                        onefile_statement +
                        include_paths_cmd_line + [
                "--no-union-value-namespacing",
                "--gen-object-api",
                src.path,
            ],
            progress_message = "Generating flatbuffer files for {}:".format(src),
            use_default_shell_env = True,
        )
    return [
        DefaultInfo(files = depset(outputs)),
    ]


_gen_flatbuffer_srcs = rule(
    _gen_flatbuffer_srcs_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".fbs"],
            mandatory = True,
        ),
        "outputs": attr.string_list(
            default = [],
            mandatory = False,
        ),
        "deps": attr.label_list(
            default = [],
            mandatory = False,
            aspects = [_flatbuffer_schemas_aspect],
        ),
        "include_paths": attr.string_list(
            default = [],
            mandatory = False,
        ),
        "language_flag": attr.string(
            mandatory = True,
        ),
        "no_includes": attr.bool(
            default = False,
            mandatory = False,
        ),
        "_flatc": attr.label(
            default = Label("//third_party/flatbuffers:flatc"),
            executable = True,
            cfg = "host",
        ),
    },
    output_to_genfiles = True,
)

def flatbuffer_py_strip_prefix_srcs(name, srcs = [], strip_prefix = ""):
    """Strips path prefix.
    Args:
      name: Rule name. (required)
      srcs: Source .py files. (required)
      strip_prefix: Path that needs to be stripped from the srcs filepaths. (required)
    """
    for src in srcs:
        native.genrule(
            name = name + "_" + src.replace(".", "_").replace("/", "_"),
            srcs = [src],
            outs = [src.replace(strip_prefix, "")],
            cmd = "cp $< $@",
        )

def _concat_flatbuffer_py_srcs_impl(ctx):
    # Merge all generated python files. The files are concatenated and import
    # statements are removed. Finally we import the flatbuffer runtime library.
    # IMPORTANT: Our Windows shell does not support "find ... -exec" properly.
    # If you're changing the commandline below, please build wheels and run smoke
    # tests on all the three operating systems.
    command = "echo 'import flatbuffers\n' > %s; "
    command += "for f in $(find %s -name '*.py' | sort); do cat $f | sed '/import flatbuffers/d' >> %s; done "
    ctx.actions.run_shell(
        inputs = ctx.attr.deps[0].files,
        outputs = [ctx.outputs.out],
        command = command % (
            ctx.outputs.out.path,
            ctx.attr.deps[0].files.to_list()[0].path,
            ctx.outputs.out.path,
        ),
        use_default_shell_env = True,
    )

_concat_flatbuffer_py_srcs = rule(
    _concat_flatbuffer_py_srcs_impl,
    attrs = {
        "deps": attr.label_list(mandatory = True),
    },
    output_to_genfiles = True,
    outputs = {"out": "%{name}.py"},
)

def flatbuffer_py_library(
        name,
        srcs,
        deps = [],
        include_paths = []):
    """A py_library with the generated reader/writers for the given schema.
    This rule assumes that the schema files define non-conflicting names, so that
    they can be merged in a single file. This is e.g. the case if only a single
    namespace is used.
    The rule call the flatbuffer compiler for all schema files and merges the
    generated python files into a single file that is wrapped in a py_library.
    Args:
      name: Rule name. (required)
      srcs: List of source .fbs files. (required)
      deps: List of dependencies.
      include_paths: Optional, list of paths the includes files can be found in.
    """
    all_srcs = "{}_srcs".format(name)
    _gen_flatbuffer_srcs(
        name = all_srcs,
        srcs = srcs,
        language_flag = "--python",
        deps = deps,
        include_paths = include_paths,
    )

    # TODO(b/235550563): Remove the concatnation rule with 2.0.6 update.
    all_srcs_no_include = "{}_srcs_no_include".format(name)
    _gen_flatbuffer_srcs(
        name = all_srcs_no_include,
        srcs = srcs,
        language_flag = "--python",
        deps = deps,
        no_includes = True,
        include_paths = include_paths,
    )
    concat_py_srcs = "{}_generated".format(name)
    _concat_flatbuffer_py_srcs(
        name = concat_py_srcs,
        deps = [
            ":{}".format(all_srcs_no_include),
        ],
    )
    native.py_library(
        name = name,
        srcs = [
            ":{}".format(concat_py_srcs),
        ],
        srcs_version = "PY3",
        deps = deps + [
            "//third_party/flatbuffers:runtime_py",
        ],
    )
