#!/usr/bin/env bash
set -o pipefail
cd "$(dirname "$0")"
PYTHONPATH=../dpcore python3 -B ./protogen-builder.py | rustfmt > src/messages_ffi.rs && echo "Regenerated messages_ffi.rs"
