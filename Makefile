a := activate_desktop_ini

libs += kernel32
libs += shell32
libs := $(foreach lib,$(libs),$(lib).lib)

cflags += -c
cflags += -Oz
cflags += -nostdlib 
cflags += -ffreestanding
cflags += -fno-stack-check
cflags += -fno-stack-protector
cflags += -mno-stack-arg-probe

lflags += /entry:mainCRTStartup
lflags += /nodefaultlib
lflags += /subsystem:console

vswhere_path := vendor/vswhere/vswhere.exe
vswhere_args += -latest
vswhere_args += -products *
vswhere_args += -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
vswhere_args += -property installationPath

vsinstallpath := $(shell "$(vswhere_path)" $(vswhere_args))
with_vcvars   := "$(vsinstallpath)/VC/Auxiliary/Build/vcvars64.bat" &&

rcedit := "vendor/rcedit/rcedit-x64.exe"

build: bin/$a.tiny.exe bin/$a.exe
	-

bin:
	mkdir bin

bin/$a.obj: $a.c bin
	clang $< $(cflags) -o $@

bin/$a.tiny.exe: bin/$a.obj
	$(with_vcvars) link $< $(libs) $(lflags) /align:16 /out:$@

bin/$a.exe: bin/$a.obj
	$(with_vcvars) link $< $(libs) $(lflags) /out:$@
	$(rcedit) $@ --set-icon res/icon.ico

.PHONY: build
