
#!/bin/bash
set -e

# Enable GCC 11 if available
source /opt/rh/gcc-toolset-11/enable

GCC_VERSION=$(gcc -dumpversion | cut -d. -f1)
if [ "$GCC_VERSION" != "11" ]; then
    echo "ERROR: GCC 11 is required but found $(gcc --version | head -n1)"
    exit 1
fi

# Define paths inside container
TREESCAN_SRC="/workspace"
BOOST_SRC="/opt/boost"
OUTPUT_DIR="/workspace/build/binaries/linux"

# Create output directory if needed
mkdir -p "$OUTPUT_DIR"

# Move into repo root
cd /workspace/scripts/linux

# Ensure scripts are executable
chmod +x build_binaries makescript.sh makescript.so.sh

echo "Starting build..."

./build_binaries "$TREESCAN_SRC" "$BOOST_SRC" "$OUTPUT_DIR"

# Print any errors
if [ -f build.stderr ]; then
    echo "---- build.stderr ----"
    cat build.stderr
fi

echo "Build complete."