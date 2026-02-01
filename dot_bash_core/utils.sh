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

find_shuf() {
    find . -type f -print0 | shuf -z | xargs -0 mpv
}

find_cpy() {
    find . -type f -exec cp -n {} "${1?missing target dir}" \;
}

bcs() {
    git branch --sort=-committerdate
}

plist() {
    local pat="${1}"
    if [[ -z "$pat" ]]; then
        read -r pat
    fi
    pss aux | rg "$(whoami)" | rg "$pat"
}

plistkill() {
    local pat="${1?missing pattern}"; shift
    local sig="${1:-10}"
    pss aux | rg "$(whoami)" | rg "$pat" | awk '{print $2}' | xargs kill "${sig}"
}

ppkill() {
    plist "$@" | awk '{print $2}' | xargs kill
}

uuidv4() {
    uuidgen | tr -d '\n' | tr '[:upper:]' '[:lower:]'
}

extract_frames() {
    local input="$1"
    local fps="${2:-0}"
    local basename="${input%.*}"
    local output="${basename}_frame_%05d.png"
    if [ "$fps" = "0" ]; then
        ffmpeg -i "$input" -c:v png -vsync 0 "$output"
    else
        ffmpeg -i "$input" -vf "fps=$fps" -c:v png "$output"
    fi
}

lh() {
    ls -liht | head "$@"
}

millis() {
    local millis=$1;
    if [[ -z $millis ]]; then read -r millis; fi
    local -i secs=$((millis/1000))
    local -i sub=$((millis % 1000))
    local dp=
    dp=$(date -d "@$secs" '+%Y-%m-%d %H:%M:%S')
    echo "$dp.$(printf '%03d' $sub)"
}

secs() {
    local secs=$1; if [[ -z "$secs" ]]; then read -r secs; fi
    if [[ -z $secs ]]; then
        echo "Usage: $0 <seconds>" >&2
        return 1
    else
        date -d@"$secs"
    fi
}

nanos() {
    local nanos=$1; if [[ -z "$nanos" ]]; then read -r nanos; fi
    [[ -z $nanos ]] && { echo "Usage: $0 <nanos>" >&2; return 1 ; } ;
    date -d@$(( nanos / (1000 * 1000 * 1000) )).$(( nanos % (1000 * 1000 * 1000) ))
}

now() {
    date +%s
}

nowms() {
    date +%s%3N
}

nowns() {
    date +%s%N
}

ignore_untracked() {
    local exclude_file
    local repo_root

    if ! git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
        echo "not in a git repository"
        return 1
    fi

    repo_root=$(git rev-parse --show-toplevel)
    exclude_file="$repo_root/.git/info/exclude"
    (
        cd "$repo_root" || exit 1
        git status --porcelain | grep '^??' | sed 's/^?? //' | while read -r file; do
            echo "${file}" >> "$exclude_file"
        done

    )
    echo "added untracked files to $exclude_file" >&2
}
