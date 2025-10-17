#!/bin/bash

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

install_neovim_arch() {
    sudo pacman -Syu nvim
}

install_neovim_centos() {
    cd /tmp
    curl -LO https://github.com/neovim/neovim-releases/releases/download/v0.11.4/nvim-linux-x86_64.tar.gz
    tar -zxzf nvim-linux-x86_64.tar.gz
    mv nvim-linux-x86_64 "$HOME/.local/nvim"
    ln -sf "$HOME/.local/nvim/bin/nvim" "$BIN_DIR/nvim"
}

install_rustup() {
    if command -v rustup >/dev/null 2>&1; then
        echo "rustup already installed"
        return 0
    fi

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --no-modify-path
    . "${CARGO_HOME:-$HOME/.cargo}/env"
}

install_cargo_tools() {
    if ! command -v cargo >/dev/null 2>&1; then
        echo "cargo not found, installing rustup first"
        install_rustup
    fi

    cargo install ripgrep fd-find bat zellij
}

install_fzf() {
    if [[ -f  "$HOME/.fzf" ]]; then
        echo "fzf already installed" >&2
        return 0
    fi
    git clone --depth 1 https://github.com/junegunn/fzf.git "$HOME/.fzf"
    "$HOME/.fzf/install" --bin
    ln -sf "$HOME/.fzf/bin/fzf" "$BIN_DIR/fzf"
}

install_ruby() {
    git clone https://github.com/rbenv/rbenv.git ~/.rbenv
    git clone https://github.com/rbenv/ruby-build.git ~/.rbenv/plugins/ruby-build
    rbenv install 3.3.5
    rbenv global 3.3.5
    gem install --user-install ruby-lsp rubocop cookstyle
    export PATH="$HOME/.rbenv/bin:$HOME/.local/share/gem/ruby/3.3.0/bin:$PATH"
    if ! [[ -d "$HOME/.rbenv" ]]; then
        install
    fi
    eval "$(rbenv init - bash)"
}

main() {
    install_neovim_centos
    install_rustup
    install_cargo_tools
    [[ -n $INSTALL_FZF ]] && install_fzf
    [[ -n $INSTALL_RUBY ]] && install_ruby
    return 0
}

main "$@"
