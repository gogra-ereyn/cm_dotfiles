#!/bin/bash

_btdev() {
    local pat="${1?missing pattern}"
    local matches
    local count

    matches="$(bluetoothctl devices | rg -i "$pat")"
    count="$(printf '%s\n' "$matches" | sed '/^$/d' | wc -l)"

    if [[ "$count" -eq 0 ]]; then
        printf 'no bluetooth device matched: %s\n' "$pat" >&2
        return 1
    fi

    if [[ "$count" -gt 1 ]]; then
        printf 'multiple bluetooth devices matched: %s\n' "$pat" >&2
        printf '%s\n' "$matches" >&2
        return 1
    fi

    printf '%s\n' "$matches" | awk '{print $2}'
}


connect() {
    local pat="${1?missing pattern}"
    bluetoothctl connect "$(_btdev "$pat")"
}

disconnect() {
    local pat="${1:-}"
    if [[ -z $pat ]]; then
        bluetoothctl disconnect
        return
    fi
    bluetoothctl disconnect "$(_btdev "$pat")"
}
