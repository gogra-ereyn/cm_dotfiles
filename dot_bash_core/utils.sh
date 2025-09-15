#!/bin/bash

take() {
    local dir="${1?Missing dir name}"
    mkdir "$dir" && cd "$dir"
}

fpv() {
    pv -bartF '%t %a %r %b' > /dev/null
}

filepv() {
    pv -f -i 1 -F '%t %a %r %b' >/dev/null 2> >(stdbuf -o0 tr '\r' '\n' > ${OUTFILE:-./pv_out.log})
}

# Newline separated pv output; prepend each line with millisecond timestamp
timepv() {
    pv -f -i 1 -N c1 -F '%N %t %a %r %b' >/dev/null   2> >(stdbuf -o0 tr '\r' '\n' | while IFS= read -r l; do ms=${EPOCHREALTIME/./}; printf '%.*s %s\n' 13 "$ms" "$l"; done > c1.pv.log)

}

pss() {
    ps --sort=start_time "$@"
}

