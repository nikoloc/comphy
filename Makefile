.PHONY: clean
build/comphy: build/toplevel.o build/desktop-shell.o build/comphy.o | build
	cc -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -o build/comphy build/toplevel.o build/desktop-shell.o build/comphy.o
clean: Makefile | build
	rm -rf build Makefile
build/toplevel.o: src/toplevel.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -o build/toplevel.o -MMD -MP -MF build/deps/toplevel.d src/toplevel.c
-include build/deps/toplevel.d
build/desktop-shell.o: src/desktop-shell.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -o build/desktop-shell.o -MMD -MP -MF build/deps/desktop-shell.d src/desktop-shell.c
-include build/deps/desktop-shell.d
build/comphy.o: src/comphy.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -o build/comphy.o -MMD -MP -MF build/deps/comphy.d src/comphy.c
-include build/deps/comphy.d
