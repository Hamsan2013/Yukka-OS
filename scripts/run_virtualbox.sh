#!/usr/bin/env bash
set -euo pipefail

VM_NAME="Yukka OS"

VBoxManage createvm --name "$VM_NAME" --ostype Other_64 --register || true
VBoxManage storagectl "$VM_NAME" --name "IDE" --add ide || true
VBoxManage storageattach "$VM_NAME" --storagectl "IDE" --port 0 --device 0 \
    --type dvddrive --medium Yukka-OS.iso
VBoxManage startvm "$VM_NAME"
