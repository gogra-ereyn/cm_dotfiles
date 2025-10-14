#!/bin/sh

PREFIX="$HOME/.local"
BIN_DIR="$PREFIX/bin"

install_from_source() {
    NAME="$1"
    URL="$2"

    cd /tmp
    curl -L "$URL" | tar xz
    cd "$NAME"*
    ./configure --prefix="$PREFIX"
    make
    make install
    cd ..
    rm -rf "$NAME"*
}

install_neovim_centos() {
    cd /tmp
    curl -LO https://github.com/neovim/neovim/releases/download/stable/nvim-linux64.tar.gz
    tar xzf nvim-linux64.tar.gz
    mv nvim-linux64 "$HOME/.local/nvim"
    ln -sf "$HOME/.local/nvim/bin/nvim" "$BIN_DIR/nvim"
}

install_rustup() {
    if command -v rustup >/dev/null 2>&1; then
        echo "rustup already installed"
        return 0
    fi

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --no-modify-path
    . "$CARGO_HOME/env"
}

install_cargo_tools() {
    if ! command -v cargo >/dev/null 2>&1; then
        echo "cargo not found, installing rustup first"
        install_rustup
    fi

    cargo install ripgrep fd-find bat
}

install_fzf() {
    git clone --depth 1 https://github.com/junegunn/fzf.git "$HOME/.fzf"
    "$HOME/.fzf/install" --bin
    ln -sf "$HOME/.fzf/bin/fzf" "$BIN_DIR/fzf"
}

main() {
    install_rustup
    install_cargo_tools
    install_fzf
    install_neovim_centos
}

main "$@"
