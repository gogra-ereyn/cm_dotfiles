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

comparepid() {
    pidstat -rud -h -p ${1:?missing pid1},${2:?missing pid2} 1 | tee ${3:-pidstat.log}
}


pss() {
    ps --sort=start_time "$@"
}

print_zlen() {
    find . -type f -size 0b -print
}

delete_zlen() {
    find . -type f -size 0b -delete
}

bcs() {
    git branch --sort=-committerdate
}

