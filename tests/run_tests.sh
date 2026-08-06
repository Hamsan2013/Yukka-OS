#!/usr/bin/env bash
set -euo pipefail

ISO="Yukka-OS.iso"
LOG="build/qemu-test.log"

mkdir -p build

if [ ! -f "$ISO" ]; then
    echo "Missing $ISO"
    exit 1
fi

timeout 30 qemu-system-x86_64 \
    -cdrom "$ISO" \
    -display none \
    -serial file:"$LOG" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
    -no-reboot || true

grep -q "Yukka OS" "$LOG"
grep -q "TEST PASS boot" "$LOG"
grep -q "TEST PASS memory" "$LOG"
grep -q "TEST PASS filesystem" "$LOG"
grep -q "TEST PASS terminal" "$LOG"
grep -q "TEST PASS yakka" "$LOG"

echo "ALL TESTS PASSED"
