from aods import (
    Context,
    run,
    default_release_flags,
    default_debug_flags,
)

import sys
import os


def init_protocols(dest: str):
    protocol_files = ["protocols/" + f for f in os.listdir("protocols")]

    for file in protocol_files:
        name = file.split("/")[-1]
        name = name.replace("-", "_")

        header = dest + "/" + name.replace(".xml", "_protocol.h")

        ok, output = run(["wayland-scanner", "server-header", file, header])
        if not ok:
            raise Exception(
                f"wayland protocol initialization failed for `{file}`\nreason: {output}"
            )


def mkdir_or_pass(path: str):
    try:
        os.mkdir(path)
    except:
        pass


release_build = "--release" in sys.argv

mkdir_or_pass("generated")
init_protocols("generated")

ctx = Context("comphy")

ctx.add_include(
    [
        "include",
        "util",
    ]
)

ctx.add_dependency(
    [
        "wayland-server",
        "wlroots-0.19",
        "xkbcommon",
    ]
)

ctx.add_source(["src/" + f for f in os.listdir("src")])

if release_build:
    ctx.add_flag(default_release_flags())
else:
    ctx.add_flag(default_debug_flags())

ctx.add_flag(
    [
        "-lm",  # link to the math library
        "-std=c99",  # use the c99 standard
        "-DWLR_USE_UNSTABLE",  # use the c99 standard
        "-D_GNU_SOURCE",  # dont care about portability for now
    ]
)

ctx.build()
