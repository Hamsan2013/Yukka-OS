#!/usr/bin/env bash
set -euo pipefail

cat > build/Yukka-OS.vmx <<'EOF'
displayName = "Yukka OS"
guestOS = "other-64"
memsize = "128"
ide0:0.present = "TRUE"
ide0:0.deviceType = "cdrom-image"
ide0:0.fileName = "Yukka-OS.iso"
EOF

echo "Created build/Yukka-OS.vmx"
