# Yukka OS

Yukka OS is a terminal-only x86_64 operating system.

- Bootloader: GRUB2
- Kernel: hybrid, C GNU11
- System software: C++20
- Terminal language: Yakka
- Targets: QEMU, VirtualBox, VMware

## Build

```sh
make iso
```

## Run

```sh
make run
```

## Test

```sh
make test
```

## Artifacts

The build produces:

```text
Yukka-OS.iso
```
