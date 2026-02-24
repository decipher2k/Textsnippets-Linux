#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="1.0.0"
PKG_NAME="textbaustein-manager"

echo "========================================="
echo " Textbaustein-Manager .deb Package Builder"
echo "========================================="
echo ""

# ---------- Install cross-compilation tools ----------
echo "[1/6] Installing cross-compilation tools for amd64..."
sudo apt-get update -qq
sudo apt-get install -y \
    g++-x86-64-linux-gnu \
    libgtk-3-dev:amd64 \
    libayatana-appindicator3-dev:amd64 \
    pkg-config \
    dpkg-dev

# Ensure native arm64 build deps are installed
echo "[2/6] Ensuring native arm64 build dependencies..."
sudo apt-get install -y \
    build-essential cmake pkg-config \
    libgtk-3-dev libayatana-appindicator3-dev \
    nlohmann-json3-dev

# ---------- Build arm64 (native) ----------
echo ""
echo "[3/6] Building arm64 (native)..."
rm -rf build-arm64
cmake -B build-arm64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm64 -j$(nproc)

# ---------- Package arm64 ----------
echo "[4/6] Packaging arm64 .deb..."
cd build-arm64
cpack -G DEB
cd "$SCRIPT_DIR"

# ---------- Build amd64 (cross-compile) ----------
echo ""
echo "[5/6] Building amd64 (cross-compile)..."
rm -rf build-amd64
cmake -B build-amd64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/cmake/toolchain-amd64.cmake"
cmake --build build-amd64 -j$(nproc)

# ---------- Package amd64 ----------
echo "[6/6] Packaging amd64 .deb..."
cd build-amd64
cpack -G DEB
cd "$SCRIPT_DIR"

# ---------- Summary ----------
echo ""
echo "========================================="
echo " Build complete!"
echo "========================================="
echo ""
echo "Packages created:"
ls -lh build-arm64/${PKG_NAME}_${VERSION}_arm64.deb 2>/dev/null || echo "  arm64: FAILED"
ls -lh build-amd64/${PKG_NAME}_${VERSION}_amd64.deb 2>/dev/null || echo "  amd64: FAILED"
echo ""
echo "Install with: sudo dpkg -i <package>.deb"
