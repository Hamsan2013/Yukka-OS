#!/usr/bin/env bash
set -euo pipefail

if command -v x86_64-elf-gcc >/dev/null 2>&1; then
    echo "Cross compiler found."
    exit 0
fi

echo "Cross compiler not found. Using host compiler fallback."
