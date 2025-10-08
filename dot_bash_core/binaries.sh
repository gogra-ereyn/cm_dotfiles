# rust
# set CARGO_HOME and RUSTUP_HOME to desired dirs
rustup_install() {
     curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
}

# fzf
fzf_install() {
    git clone --depth 1 https://github.com/junegunn/fzf.git ~/.local/fzf
    ~/.local/fzf/install
}


# chezmoi

