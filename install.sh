#!/usr/bin/env bash
set -euo pipefail

PROG_NAME="mini_fzf_fast"
SRC="mini_fzf_fast.c"
INSTALL_PREFIX="/usr/local"
INSTALL_PATH="${INSTALL_PREFIX}/bin/${PROG_NAME}"

# --- Detect OS & install dependencies ---
echo "Checking dependencies..."

if command -v apt-get >/dev/null 2>&1; then
    echo "Installing dependencies via apt..."
    sudo apt-get update
    sudo apt-get install -y build-essential pkg-config libncurses5-dev libncursesw5-dev
elif command -v dnf >/dev/null 2>&1; then
    echo "Installing dependencies via dnf..."
    sudo dnf install -y gcc make pkg-config ncurses-devel
elif command -v pacman >/dev/null 2>&1; then
    echo "Installing dependencies via pacman..."
    sudo pacman -Sy --noconfirm base-devel ncurses pkgconf
elif [[ "$(uname)" == "Darwin" ]]; then
    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew not found. Please install it first: https://brew.sh/"
        exit 1
    fi
    echo "Installing dependencies via brew..."
    brew install ncurses pkg-config
else
    echo "Unsupported OS. Please install gcc, ncurses, and pkg-config manually."
fi

# --- Build ---
echo "Building ${PROG_NAME}..."
PKG_CFLAGS=$(pkg-config --cflags ncursesw 2>/dev/null || pkg-config --cflags ncurses 2>/dev/null || echo "")
PKG_LIBS=$(pkg-config --libs ncursesw 2>/dev/null || pkg-config --libs ncurses 2>/dev/null || echo "-lncurses")

gcc ${PKG_CFLAGS} -o "${PROG_NAME}" "${SRC}" ${PKG_LIBS}
echo "Build completed: ./${PROG_NAME}"

# --- Install ---
echo "Installing to ${INSTALL_PATH} (sudo may be required)..."
sudo mkdir -p "$(dirname "${INSTALL_PATH}")"
sudo cp -f "./${PROG_NAME}" "${INSTALL_PATH}"
sudo chmod 755 "${INSTALL_PATH}"

echo "✅ Installation complete. You can now run '${PROG_NAME}' from anywhere."
