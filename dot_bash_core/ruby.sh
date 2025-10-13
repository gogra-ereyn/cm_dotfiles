#/bin/bash


install() {
    git clone https://github.com/rbenv/rbenv.git ~/.rbenv
    git clone https://github.com/rbenv/ruby-build.git ~/.rbenv/plugins/ruby-build
    rbenv install 3.3.5
    rbenv global 3.3.5
    gem install --user-install ruby-lsp  rubocop cookstyle
}


main() {
    export PATH="$HOME/.rbenv/bin:$HOME/ereyn/.local/share/gem/ruby/3.3.0/bin:$PATH"
    if ! [[ -d "$HOME/.rbenv" ]]; then
        install
    fi
    eval "$(rbenv init - bash)"
}


main "$@"
