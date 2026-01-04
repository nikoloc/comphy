.PHONY: clean
build/comphy: build/scene.o build/backend.o build/toplevel.o build/desktop-shell.o build/comphy.o | build
	cc -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy build/scene.o build/backend.o build/toplevel.o build/desktop-shell.o build/comphy.o
clean: Makefile | build
	rm -rf build Makefile
build/scene.o: src/scene.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/scene.o -MMD -MP -MF build/deps/scene.d src/scene.c
-include build/deps/scene.d
build/backend.o: src/backend.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/backend.o -MMD -MP -MF build/deps/backend.d src/backend.c
-include build/deps/backend.d
build/toplevel.o: src/toplevel.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/toplevel.o -MMD -MP -MF build/deps/toplevel.d src/toplevel.c
-include build/deps/toplevel.d
build/desktop-shell.o: src/desktop-shell.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/desktop-shell.o -MMD -MP -MF build/deps/desktop-shell.d src/desktop-shell.c
-include build/deps/desktop-shell.d
build/comphy.o: src/comphy.c | build build/deps
	cc -c -Iinclude -Iutil -I/usr/include/wlroots-0.19 -I/usr/include/pixman-1 -I/usr/include/libdrm -lwlroots-0.19 -lwayland-server -lm -lxkbcommon -g -O0 -Wall -Wextra -Wpedantic -Wnull-dereference -fsanitize=address,undefined -fno-omit-frame-pointer -lm -std=c99 -DWLR_USE_UNSTABLE -D_GNU_SOURCE -o build/comphy.o -MMD -MP -MF build/deps/comphy.d src/comphy.c
-include build/deps/comphy.d
