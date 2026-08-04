from aods import (
    Context,
    run,
    release_flags,
    debug_flags,
)

import sys
import os


def init_protocols(src: str, dest: str):
    protocol_files = [f"{src}/{f}" for f in os.listdir(src)]

    for file in protocol_files:
        name = file.split("/")[-1]

        header = dest + "/" + name.replace(".xml", "-protocol.h")

        ok, output = run(["wayland-scanner", "server-header", file, header])
        if not ok:
            raise Exception(
                f"wayland protocol initialization failed for `{file}`: {output}"
            )


def mkdir_or_pass(path: str):
    try:
        os.mkdir(path)
    except:
        pass


release_build = "--release" in sys.argv

mkdir_or_pass("generated")
init_protocols("protocols", "generated")

ctx = Context("comphy")

ctx.add_include(
    [
        ".",
        "include",
        "generated",
    ]
)

ctx.add_dependency(
    [
        "wayland-server",
        "wlroots-0.20",
        "xkbcommon",
        "libinput",
    ]
)

ctx.add_source(
    [
        "src/action.c",
        "src/action_parser.c",
        "src/backend.c",
        "src/color.c",
        "src/comphy.c",
        "src/config.c",
        "src/ctl.c",
        "src/cursor.c",
        "src/decoration.c",
        "src/gamma_control.c",
        "src/keyboard.c",
        "src/layer_shell.c",
        "src/layout.c",
        "src/lock.c",
        "src/operation.c",
        "src/output.c",
        "src/pointer.c",
        "src/popup.c",
        "src/scene.c",
        "src/seat.c",
        "src/state.c",
        "src/system.c",
        "src/toplevel.c",
        "src/transaction.c",
        "src/view.c",
        "src/workspace.c",
        "src/xdg_shell.c",
    ]
)


if release_build:
    ctx.add_flag(release_flags())
else:
    ctx.add_flag(debug_flags())
    ctx.add_flag("-DDEBUG")

ctx.add_flag(
    [
        "-lm",  # link to the math library
        "-DWLR_USE_UNSTABLE",
        "-D_GNU_SOURCE",  # dont care about portability for now
    ]
)

ctx.build()
