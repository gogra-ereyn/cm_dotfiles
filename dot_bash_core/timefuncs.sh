#!/bin/bash

millis() {
    local -i ts=$1; if [[ -z $ts ]]; then read ts; fi
    local secs=$((ts / 1000))
    local ms=$((ts % 1000))
    local dp=
    dp="$(date -d "@$secs" '+%Y-%m-%d %H:%M:%S')"
    echo "$dp.$(printf '%03d' $ms)"
}

today() {
    date +%Y-%m-%d
}
