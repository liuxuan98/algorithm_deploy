#!/bin/bash

# Script to run pack_models tool with correct library paths
# Usage: ./run_pack_models.sh [pack_models arguments]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Set library paths
export LD_LIBRARY_PATH="$BUILD_DIR/target/Linux/lib:$SCRIPT_DIR/third_party/openvino/Linux/x86_64/lib/intel64:$SCRIPT_DIR/third_party/RSLog/Linux/lib:$LD_LIBRARY_PATH"

# Run pack_models with all arguments
exec "$BUILD_DIR/bin/pack_models" "$@" 