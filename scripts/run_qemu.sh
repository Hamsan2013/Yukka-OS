#!/usr/bin/env bash
set -euo pipefail

qemu-system-x86_64 -cdrom Yukka-OS.iso -serial stdio
