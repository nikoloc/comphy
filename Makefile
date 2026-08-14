.PHONY: clean
build/comphy: build/_objects/comphy.action.o build/_objects/comphy.action_parser.o build/_objects/comphy.backend.o build/_objects/comphy.color.o build/_objects/comphy.comphy.o build/_objects/comphy.config.o build/_objects/comphy.ctl.o build/_objects/comphy.cursor.o build/_objects/comphy.decoration.o build/_objects/comphy.gamma_control.o build/_objects/comphy.keybind.o build/_objects/comphy.keyboard.o build/_objects/comphy.layer.o build/_objects/comphy.layer_shell.o build/_objects/comphy.layout.o build/_objects/comphy.lock.o build/_objects/comphy.operation.o build/_objects/comphy.output.o build/_objects/comphy.pointer.o build/_objects/comphy.popup.o build/_objects/comphy.rules.o build/_objects/comphy.scene.o build/_objects/comphy.seat.o build/_objects/comphy.state.o build/_objects/comphy.system.o build/_objects/comphy.toplevel.o build/_objects/comphy.transaction.o build/_objects/comphy.view.o build/_objects/comphy.workspace.o build/_objects/comphy.xdg_shell.o
	cc -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy build/_objects/comphy.action.o build/_objects/comphy.action_parser.o build/_objects/comphy.backend.o build/_objects/comphy.color.o build/_objects/comphy.comphy.o build/_objects/comphy.config.o build/_objects/comphy.ctl.o build/_objects/comphy.cursor.o build/_objects/comphy.decoration.o build/_objects/comphy.gamma_control.o build/_objects/comphy.keybind.o build/_objects/comphy.keyboard.o build/_objects/comphy.layer.o build/_objects/comphy.layer_shell.o build/_objects/comphy.layout.o build/_objects/comphy.lock.o build/_objects/comphy.operation.o build/_objects/comphy.output.o build/_objects/comphy.pointer.o build/_objects/comphy.popup.o build/_objects/comphy.rules.o build/_objects/comphy.scene.o build/_objects/comphy.seat.o build/_objects/comphy.state.o build/_objects/comphy.system.o build/_objects/comphy.toplevel.o build/_objects/comphy.transaction.o build/_objects/comphy.view.o build/_objects/comphy.workspace.o build/_objects/comphy.xdg_shell.o
clean: Makefile
	rm -rf build Makefile
build/_objects/comphy.action.o: src/action.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.action.o -MMD -MP -MF build/_deps/comphy.action.d src/action.c
-include build/_deps/comphy.action.d
build/_objects/comphy.action_parser.o: src/action_parser.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.action_parser.o -MMD -MP -MF build/_deps/comphy.action_parser.d src/action_parser.c
-include build/_deps/comphy.action_parser.d
build/_objects/comphy.backend.o: src/backend.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.backend.o -MMD -MP -MF build/_deps/comphy.backend.d src/backend.c
-include build/_deps/comphy.backend.d
build/_objects/comphy.color.o: src/color.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.color.o -MMD -MP -MF build/_deps/comphy.color.d src/color.c
-include build/_deps/comphy.color.d
build/_objects/comphy.comphy.o: src/comphy.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.comphy.o -MMD -MP -MF build/_deps/comphy.comphy.d src/comphy.c
-include build/_deps/comphy.comphy.d
build/_objects/comphy.config.o: src/config.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.config.o -MMD -MP -MF build/_deps/comphy.config.d src/config.c
-include build/_deps/comphy.config.d
build/_objects/comphy.ctl.o: src/ctl.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.ctl.o -MMD -MP -MF build/_deps/comphy.ctl.d src/ctl.c
-include build/_deps/comphy.ctl.d
build/_objects/comphy.cursor.o: src/cursor.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.cursor.o -MMD -MP -MF build/_deps/comphy.cursor.d src/cursor.c
-include build/_deps/comphy.cursor.d
build/_objects/comphy.decoration.o: src/decoration.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.decoration.o -MMD -MP -MF build/_deps/comphy.decoration.d src/decoration.c
-include build/_deps/comphy.decoration.d
build/_objects/comphy.gamma_control.o: src/gamma_control.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.gamma_control.o -MMD -MP -MF build/_deps/comphy.gamma_control.d src/gamma_control.c
-include build/_deps/comphy.gamma_control.d
build/_objects/comphy.keybind.o: src/keybind.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.keybind.o -MMD -MP -MF build/_deps/comphy.keybind.d src/keybind.c
-include build/_deps/comphy.keybind.d
build/_objects/comphy.keyboard.o: src/keyboard.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.keyboard.o -MMD -MP -MF build/_deps/comphy.keyboard.d src/keyboard.c
-include build/_deps/comphy.keyboard.d
build/_objects/comphy.layer.o: src/layer.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.layer.o -MMD -MP -MF build/_deps/comphy.layer.d src/layer.c
-include build/_deps/comphy.layer.d
build/_objects/comphy.layer_shell.o: src/layer_shell.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.layer_shell.o -MMD -MP -MF build/_deps/comphy.layer_shell.d src/layer_shell.c
-include build/_deps/comphy.layer_shell.d
build/_objects/comphy.layout.o: src/layout.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.layout.o -MMD -MP -MF build/_deps/comphy.layout.d src/layout.c
-include build/_deps/comphy.layout.d
build/_objects/comphy.lock.o: src/lock.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.lock.o -MMD -MP -MF build/_deps/comphy.lock.d src/lock.c
-include build/_deps/comphy.lock.d
build/_objects/comphy.operation.o: src/operation.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.operation.o -MMD -MP -MF build/_deps/comphy.operation.d src/operation.c
-include build/_deps/comphy.operation.d
build/_objects/comphy.output.o: src/output.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.output.o -MMD -MP -MF build/_deps/comphy.output.d src/output.c
-include build/_deps/comphy.output.d
build/_objects/comphy.pointer.o: src/pointer.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.pointer.o -MMD -MP -MF build/_deps/comphy.pointer.d src/pointer.c
-include build/_deps/comphy.pointer.d
build/_objects/comphy.popup.o: src/popup.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.popup.o -MMD -MP -MF build/_deps/comphy.popup.d src/popup.c
-include build/_deps/comphy.popup.d
build/_objects/comphy.rules.o: src/rules.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.rules.o -MMD -MP -MF build/_deps/comphy.rules.d src/rules.c
-include build/_deps/comphy.rules.d
build/_objects/comphy.scene.o: src/scene.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.scene.o -MMD -MP -MF build/_deps/comphy.scene.d src/scene.c
-include build/_deps/comphy.scene.d
build/_objects/comphy.seat.o: src/seat.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.seat.o -MMD -MP -MF build/_deps/comphy.seat.d src/seat.c
-include build/_deps/comphy.seat.d
build/_objects/comphy.state.o: src/state.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.state.o -MMD -MP -MF build/_deps/comphy.state.d src/state.c
-include build/_deps/comphy.state.d
build/_objects/comphy.system.o: src/system.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.system.o -MMD -MP -MF build/_deps/comphy.system.d src/system.c
-include build/_deps/comphy.system.d
build/_objects/comphy.toplevel.o: src/toplevel.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.toplevel.o -MMD -MP -MF build/_deps/comphy.toplevel.d src/toplevel.c
-include build/_deps/comphy.toplevel.d
build/_objects/comphy.transaction.o: src/transaction.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.transaction.o -MMD -MP -MF build/_deps/comphy.transaction.d src/transaction.c
-include build/_deps/comphy.transaction.d
build/_objects/comphy.view.o: src/view.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.view.o -MMD -MP -MF build/_deps/comphy.view.d src/view.c
-include build/_deps/comphy.view.d
build/_objects/comphy.workspace.o: src/workspace.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.workspace.o -MMD -MP -MF build/_deps/comphy.workspace.d src/workspace.c
-include build/_deps/comphy.workspace.d
build/_objects/comphy.xdg_shell.o: src/xdg_shell.c
	cc -c -I. -Iinclude -Igenerated -I/usr/include/wlroots-0.20 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.20 -lwayland-server -lm -lxkbcommon -linput -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG -lm -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/_objects/comphy.xdg_shell.o -MMD -MP -MF build/_deps/comphy.xdg_shell.d src/xdg_shell.c
-include build/_deps/comphy.xdg_shell.d
