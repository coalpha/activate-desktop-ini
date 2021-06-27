a := activate_desktop_ini

libs += kernel32
libs += shell32

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

rcedit := vendor/rcedit/rcedit-x64.exe

all: bin/$a.tiny.exe bin/$a.exe
	-

bin/$a.obj: $a.c bin
	clang $< $(cflags) -o $@

bin/$a.tiny.exe: bin/$a.obj
	link $< $(foreach lib,$(libs),$(lib).lib) $(lflags) /align:16 /out:$@

bin/$a.exe: bin/$a.obj
	link $< $(foreach lib,$(libs),$(lib).lib) $(lflags) /out:$@
	$(rcedit) $@ --set-icon res/icon.ico

bin:
	mkdir bin

.PHONY: run
