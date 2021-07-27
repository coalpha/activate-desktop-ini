n := activate-desktop-ini

mode := debug
ifeq ($(mode), debug)
	cflags += -g
else
	cflags += -Ofast
	cflags += -Xlinker /align:16
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

vswhere_path := vendor/vswhere/vswhere.exe
vswhere_args += -latest
vswhere_args += -products *
vswhere_args += -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
vswhere_args += -property installationPath

vsinstallpath := $(shell "$(vswhere_path)" $(vswhere_args))
with_vcvars   := "$(vsinstallpath)/VC/Auxiliary/Build/vcvars64.bat" &&

rcedit := "vendor/rcedit/rcedit-x64.exe"

make: bin/$n.tiny.exe
	-

bin:
	mkdir bin

# build: bin/$n.tiny.exe bin/$n.exe
# 	-

bin/$n.tiny.exe: $n.c bin Makefile
	clang $< $(cflags) -o $@

# bin/$n.exe: bin/$n.obj
# 	$(with_vcvars) link $< $(libs) $(lflags) /out:$@
# 	$(rcedit) $@ --set-icon res/icon.ico

.PHONY: build
