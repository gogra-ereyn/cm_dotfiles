#!/bin/bash

take() {
    local dir="${1?Missing dir name}"
    mkdir "$dir" && cd "$dir"
}

fpv() {
    pv -bartF '%t %a %r %b' > /dev/null
}

pss() {
    ps --sort=start_time "$@"
}

