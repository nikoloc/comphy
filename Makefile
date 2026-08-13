.PHONY: clean
build/comphy/comphy: build/comphy/action.o build/comphy/action_parser.o build/comphy/backend.o build/comphy/color.o build/comphy/comphy.o build/comphy/config.o build/comphy/ctl.o build/comphy/cursor.o build/comphy/decoration.o build/comphy/gamma_control.o build/comphy/keybind.o build/comphy/keyboard.o build/comphy/layer.o build/comphy/layer_shell.o build/comphy/layout.o build/comphy/lock.o build/comphy/operation.o build/comphy/output.o build/comphy/pointer.o build/comphy/popup.o build/comphy/rules.o build/comphy/scene.o build/comphy/seat.o build/comphy/state.o build/comphy/system.o build/comphy/toplevel.o build/comphy/transaction.o build/comphy/view.o build/comphy/workspace.o build/comphy/xdg_shell.o
	cc -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/comphy build/comphy/action.o build/comphy/action_parser.o build/comphy/backend.o build/comphy/color.o build/comphy/comphy.o build/comphy/config.o build/comphy/ctl.o build/comphy/cursor.o build/comphy/decoration.o build/comphy/gamma_control.o build/comphy/keybind.o build/comphy/keyboard.o build/comphy/layer.o build/comphy/layer_shell.o build/comphy/layout.o build/comphy/lock.o build/comphy/operation.o build/comphy/output.o build/comphy/pointer.o build/comphy/popup.o build/comphy/rules.o build/comphy/scene.o build/comphy/seat.o build/comphy/state.o build/comphy/system.o build/comphy/toplevel.o build/comphy/transaction.o build/comphy/view.o build/comphy/workspace.o build/comphy/xdg_shell.o
clean: Makefile
	rm -rf build Makefile
build/comphy/action.o: src/action.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/action.o -MMD -MP -MF build/comphy/_deps/action.d src/action.c
-include build/comphy/_deps/action.d
build/comphy/action_parser.o: src/action_parser.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/action_parser.o -MMD -MP -MF build/comphy/_deps/action_parser.d src/action_parser.c
-include build/comphy/_deps/action_parser.d
build/comphy/backend.o: src/backend.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/backend.o -MMD -MP -MF build/comphy/_deps/backend.d src/backend.c
-include build/comphy/_deps/backend.d
build/comphy/color.o: src/color.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/color.o -MMD -MP -MF build/comphy/_deps/color.d src/color.c
-include build/comphy/_deps/color.d
build/comphy/comphy.o: src/comphy.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/comphy.o -MMD -MP -MF build/comphy/_deps/comphy.d src/comphy.c
-include build/comphy/_deps/comphy.d
build/comphy/config.o: src/config.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/config.o -MMD -MP -MF build/comphy/_deps/config.d src/config.c
-include build/comphy/_deps/config.d
build/comphy/ctl.o: src/ctl.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/ctl.o -MMD -MP -MF build/comphy/_deps/ctl.d src/ctl.c
-include build/comphy/_deps/ctl.d
build/comphy/cursor.o: src/cursor.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/cursor.o -MMD -MP -MF build/comphy/_deps/cursor.d src/cursor.c
-include build/comphy/_deps/cursor.d
build/comphy/decoration.o: src/decoration.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/decoration.o -MMD -MP -MF build/comphy/_deps/decoration.d src/decoration.c
-include build/comphy/_deps/decoration.d
build/comphy/gamma_control.o: src/gamma_control.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/gamma_control.o -MMD -MP -MF build/comphy/_deps/gamma_control.d src/gamma_control.c
-include build/comphy/_deps/gamma_control.d
build/comphy/keybind.o: src/keybind.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/keybind.o -MMD -MP -MF build/comphy/_deps/keybind.d src/keybind.c
-include build/comphy/_deps/keybind.d
build/comphy/keyboard.o: src/keyboard.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/keyboard.o -MMD -MP -MF build/comphy/_deps/keyboard.d src/keyboard.c
-include build/comphy/_deps/keyboard.d
build/comphy/layer.o: src/layer.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/layer.o -MMD -MP -MF build/comphy/_deps/layer.d src/layer.c
-include build/comphy/_deps/layer.d
build/comphy/layer_shell.o: src/layer_shell.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/layer_shell.o -MMD -MP -MF build/comphy/_deps/layer_shell.d src/layer_shell.c
-include build/comphy/_deps/layer_shell.d
build/comphy/layout.o: src/layout.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/layout.o -MMD -MP -MF build/comphy/_deps/layout.d src/layout.c
-include build/comphy/_deps/layout.d
build/comphy/lock.o: src/lock.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/lock.o -MMD -MP -MF build/comphy/_deps/lock.d src/lock.c
-include build/comphy/_deps/lock.d
build/comphy/operation.o: src/operation.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/operation.o -MMD -MP -MF build/comphy/_deps/operation.d src/operation.c
-include build/comphy/_deps/operation.d
build/comphy/output.o: src/output.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/output.o -MMD -MP -MF build/comphy/_deps/output.d src/output.c
-include build/comphy/_deps/output.d
build/comphy/pointer.o: src/pointer.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/pointer.o -MMD -MP -MF build/comphy/_deps/pointer.d src/pointer.c
-include build/comphy/_deps/pointer.d
build/comphy/popup.o: src/popup.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/popup.o -MMD -MP -MF build/comphy/_deps/popup.d src/popup.c
-include build/comphy/_deps/popup.d
build/comphy/rules.o: src/rules.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/rules.o -MMD -MP -MF build/comphy/_deps/rules.d src/rules.c
-include build/comphy/_deps/rules.d
build/comphy/scene.o: src/scene.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/scene.o -MMD -MP -MF build/comphy/_deps/scene.d src/scene.c
-include build/comphy/_deps/scene.d
build/comphy/seat.o: src/seat.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/seat.o -MMD -MP -MF build/comphy/_deps/seat.d src/seat.c
-include build/comphy/_deps/seat.d
build/comphy/state.o: src/state.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/state.o -MMD -MP -MF build/comphy/_deps/state.d src/state.c
-include build/comphy/_deps/state.d
build/comphy/system.o: src/system.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/system.o -MMD -MP -MF build/comphy/_deps/system.d src/system.c
-include build/comphy/_deps/system.d
build/comphy/toplevel.o: src/toplevel.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/toplevel.o -MMD -MP -MF build/comphy/_deps/toplevel.d src/toplevel.c
-include build/comphy/_deps/toplevel.d
build/comphy/transaction.o: src/transaction.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/transaction.o -MMD -MP -MF build/comphy/_deps/transaction.d src/transaction.c
-include build/comphy/_deps/transaction.d
build/comphy/view.o: src/view.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/view.o -MMD -MP -MF build/comphy/_deps/view.d src/view.c
-include build/comphy/_deps/view.d
build/comphy/workspace.o: src/workspace.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/workspace.o -MMD -MP -MF build/comphy/_deps/workspace.d src/workspace.c
-include build/comphy/_deps/workspace.d
build/comphy/xdg_shell.o: src/xdg_shell.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy/xdg_shell.o -MMD -MP -MF build/comphy/_deps/xdg_shell.d src/xdg_shell.c
-include build/comphy/_deps/xdg_shell.d
