#!/bin/bash

main() {
    local -i i=0
    local sl="${1-1}"
    for ((;;)); do
        echo "line $((++i))"
        sleep "$sl"
    done
}

main "$@"

