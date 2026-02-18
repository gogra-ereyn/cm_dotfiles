#!/bin/bash

main() {
    local host=
    if [[ -r /proc/sys/kernel/hostname ]]; then
        IFS= read -r host < /proc/sys/kernel/hostname
    elif [[ -r /etc/hostname ]]; then
        IFS= read -r host < /etc/hostname
    elif command -v uname >/dev/null 2>&1; then
        host="$(uname -n 2>/dev/null)"
    else
        host="unknown"
    fi
    echo "$host"
}
main "$@"
