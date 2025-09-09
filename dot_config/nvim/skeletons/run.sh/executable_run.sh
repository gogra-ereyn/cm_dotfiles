#!/bin/bash

main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    "${sd}/build.sh" || { echo "build failed" >&2 ; exit 1 ; }
    "${sd}/build/main" "$@"
}

main "$@"
