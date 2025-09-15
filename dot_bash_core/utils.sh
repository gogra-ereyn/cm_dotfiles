#!/bin/bash

take() {
    local dir="${1?Missing dir name}"
    mkdir "$dir" && cd "$dir"
}

fpv() {
    pv -bartF '%t %a %r %b' > /dev/null
}

filepv() {
    pv -f -i 1 -F '%t %a %r %b' >/dev/null 2> >(stdbuf -o0 tr '\r' '\n' >> ${OUTFILE:-./pv_out.log})
}

pss() {
    ps --sort=start_time "$@"
}

