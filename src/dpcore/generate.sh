#!/usr/bin/env bash
set -o pipefail
cd $(dirname "$0")
python3 -B ./protogen-rust.py | rustfmt > src/protocol/message.rs && echo "Regenerated message.rs"
