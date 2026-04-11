#!/bin/bash

set -eu

main() {
    local cfg_dir="${XDG_CONFIG_HOME:-$HOME/.config}/monitors"
    local layout="$cfg_dir/layout.sh"

    mkdir -p "$cfg_dir"

    xrandr "$@"

    {
        printf '%s\n' '#!/bin/sh'
        printf '%s' 'xrandr'
        for arg in "$@"; do
            printf " '%s'" "$(printf '%s' "$arg" | sed "s/'/'\\\\''/g")"
        done
        printf '\n'
    } >"$layout"

    chmod +x "$layout"
}

main "$@"
