DESTDIR ?= sysroot
C_SOURCES := $(shell find src -name "*.c")
ASM_SOURCES := $(shell find src -name "*.S")
NASM_SOURCES := $(shell find src -name "*.asm")
HEADERS := $(shell find include -name "*.h")

OBJ := $(patsubst src/%.c,build/%.c.o,$(C_SOURCES))
OBJ += $(patsubst src/%.S,build/%.S.o,$(ASM_SOURCES))
OBJ += $(patsubst src/%.asm,build/%.asm.o,$(NASM_SOURCES))

CC = i686-elf-gcc
LD = i686-elf-ld
AS = i686-elf-gcc # i686-elf-as
NASM = nasm

GDB = i686-elf-gdb

CFLAGS = -g -O0 -ffreestanding -Wall -Wextra -fno-exceptions -m32 -Iinclude -Wno-int-to-pointer-cast
ASFLAGS = $(CFLAGS)
LDFLAGS = -g -O0 -nostdlib

.PHONY: all
all: kernel.elf

.PHONY: install
install: $(DESTDIR) kernel.elf
	mkdir -p $</boot
	cp kernel.elf $</boot

.PHONY: install-headers
install-headers: $(DESTDIR) $(HEADERS)
	mkdir -p $</include/kernel
	cp $(HEADERS) $</include/kernel

kernel.elf: ${OBJ}
	$(LD) -o $@ $(LDFLAGS) -Tlinker.ld $^

build/%.c.o: src/%.c ${HEADERS}
	mkdir -p $(@D)
	${CC} ${CFLAGS} -c $< -o $@

build/%.S.o: src/%.S
	mkdir -p $(@D)
	${AS} ${ASFLAGS} -c $< -o $@

build/%.asm.o: src/%.asm
	mkdir -p $(@D)
	${NASM} -f elf32 $< -o $@

clean:
	rm -rf build | true
	rm -rf bin | true
	rm -f kernel.elf | true
