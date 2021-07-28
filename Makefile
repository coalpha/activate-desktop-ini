n := activate-desktop-ini

mode := release
ifeq ($(mode), debug)
	cflags += -g
else
	cflags += -Ofast
endif

cflags += -nostdlib
cflags += -ffreestanding
cflags += -fno-stack-check
cflags += -fno-stack-protector
cflags += -mno-stack-arg-probe
cflags += -fuse-ld=lld-link
cflags += -lkernel32
cflags += -lshell32
cflags += -lole32
cflags += -luuid
cflags += -Xlinker /entry:start
cflags += -Xlinker /nodefaultlib
cflags += -Xlinker /subsystem:console
cflags += -Xlinker "/libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64"

rcedit := "misc/rcedit/rcedit-x64.exe"

build: clean bin/$n.tiny.exe bin/$n.exe
	-

bin/$n.tiny.exe: src/$n.c bin Makefile
	clang $< $(cflags) -o $@

bin/$n.exe: bin/$n.tiny.exe
	./misc/cp.exe $< $@
	$(rcedit) $@ --set-icon misc/icon.ico

clean:
	-rd /s /q bin

bin:
	mkdir bin

.PHONY: build clean bin
