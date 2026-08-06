ISO := Yukka-OS.iso
KERNEL := build/kernel.bin

CC ?= gcc
CXX ?= g++

CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -mno-red-zone \
          -mno-sse -mno-sse2 -mno-mmx -fno-builtin -O2 -Wall -Wextra \
          -Ikernel/include

CXXFLAGS := -std=c++20 -ffreestanding -fno-stack-protector -mno-red-zone \
            -mno-sse -mno-sse2 -mno-mmx -fno-builtin -fno-exceptions \
            -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics \
            -O2 -Wall -Wextra -Ikernel/include -Iyakka/include

ASFLAGS := -ffreestanding -O2
LDFLAGS := -nostdlib -static -T linker.ld

ifeq ($(TEST),1)
CFLAGS += -DENABLE_TESTS
CXXFLAGS += -DENABLE_TESTS
endif

C_SRCS := $(shell find kernel drivers terminal yakka apps -name '*.c')
CPP_SRCS := $(shell find kernel drivers terminal yakka apps -name '*.cpp')
ASM_SRCS := boot/x86_64/boot.S

OBJS := build/boot.o $(C_SRCS:%.c=build/%.o) $(CPP_SRCS:%.cpp=build/%.o)

.PHONY: all kernel iso run test clean

all: kernel

kernel: $(KERNEL)

$(KERNEL): $(OBJS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lgcc

build/boot.o: boot/x86_64/boot.S
	mkdir -p build
	$(CC) $(ASFLAGS) -c $< -o $@

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

iso: $(KERNEL)
	mkdir -p build/isodir/boot/grub
	cp $(KERNEL) build/isodir/boot/kernel.bin
	cp boot/grub/grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) build/isodir

run: iso
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio

test:
	$(MAKE) TEST=1 iso
	bash tests/run_tests.sh

clean:
	rm -rf build $(ISO)
