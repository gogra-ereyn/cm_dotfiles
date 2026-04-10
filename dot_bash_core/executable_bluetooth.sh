#!/bin/bash

_btdev() {
    local pat="${1?missing pattern}"
    bluetoothctl devices | rg -i "$pat" | awk '{print $2}'
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
