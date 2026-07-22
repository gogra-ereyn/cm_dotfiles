#!/bin/bash

# ndjson shenanigans

utils::list_to_csv() {
    paste -sd,
}

utils::njson_prefix_first() {
    local prefix="${1?missing prefix}"
    shift
    jq -r --arg p "$prefix" '[.[] | select(startswith($p))] | first // empty' "$@" | sort -u
}

utils::ndjson_prefix_all() {
    local prefix="${1?missing prefix}"
    shift
    jq -r --arg p "$prefix" '.[] | select(startswith($p))' "$@" | sort -u
}

ndjson_prefix_first() {
    local prefix="$1"
    shift
    jq -r --arg p "$prefix" '
      [.[] | select(startswith($p))] | first // empty | ltrimstr($p)
    ' "$@" | sort -u
}

utils::expand_csv() {
    tr ',' '\n' <"${1:-/dev/stdin}"
}

utils::expand_csv_trimmed() {
    tr ',' '\n' <"${1:-/dev/stdin}" | sed 's/^ *//;s/ *$//'
}

utils::wait_tcp() {
    local host="${1?missing 1/host}"
    local -i port=${2?missing 2/port}
    local timeout=${3:-30}
    local end=$((SECONDS + timeout))
    while ((SECONDS < end)); do
        if (exec 3<>"/dev/tcp/$host/$port") >/dev/null 2>&1; then
            exec 3>&-
            exec 3<&-
            return 0
        fi
        sleep 0.2
    done
    return 1
}

utils::repo_root() {
    git rev-parse --show-toplevel 2>/dev/null || pwd
}

utils::read_sleep() {
    local IFS dur
    [[ -z ${1:-} ]] && {
        echo "read_sleep: missing operand" >&2
        return 1
    }
    [[ -n "${_read_sleep_fd:-}" ]] || { exec {_read_sleep_fd}<> <(:) && read -r -t 0 -u $_read_sleep_fd; }
    [[ $1 != inf* ]] && dur="$1"
    read -r ${dur:+-t "$dur"} -u "$_read_sleep_fd" || :
}

utils::assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"
    if [ "$actual" = "$expected" ]; then
        echo -e "\033[0;32m✓\033[0m $message" >&2
        return 0
    else
        echo -e "\033[0;31m✗\033[0m $message" >&2
        echo -e "Expected: '$expected'" >&2
        echo -e "Actual:   '$actual'" >&2
        return 1
    fi
}

utils::assert_eq() {
    assert "$1" "$2" "$3"
}

utils::log() {
    (($# < 2)) && {
        echo "usage: log LEVEL message..." >&2
        return 1
    }
    local level="$1"
    shift
    printf '[%s] [%s] %s:%s:%d: %s\n' \
        "$(date +"%Y-%m-%d %H:%M:%S")" \
        "$level" \
        "${BASH_SOURCE[1]##*/}" \
        "${FUNCNAME[1]}" \
        "${BASH_LINENO[0]}" \
        "$*" >&2
}

utils::hex_to_rgb() {
    : "${1/\#/}"
    ((r = 16#${_:0:2}, g = 16#${_:2:2}, b = 16#${_:4:2}))
    printf '%s\n' "$r $g $b"
}

utils::info() {
    log "$1" info
}

utils::abort() {
    local msg="$1"
    local -i ec="${2-1}"
    log "$msg" fatal
    exit $ec
}

##
## LEGACY UNSCOPED ##
##

take() {
    local dir="${1?Missing dir name}"
    mkdir "$dir" && cd "$dir"
}

fpv() {
    pv -bartF '%t %a %r %b' >/dev/null
}

gfind_cwd() {
    local -i days=${1-14}
    find . -maxdepth 1 -type f -mtime +${days}
}

delfind_cwd() {
    local -i days=${1-14}
    find . -maxdepth 1 -type f -mtime +${days} -delete
}

filepv() {
    pv -f -i 1 -F '%t %a %r %b' >/dev/null 2> >(stdbuf -o0 tr '\r' '\n' >${OUTFILE:-./pv_out.log})
}

# Newline separated pv output; prepend each line with millisecond timestamp
timepv() {
    pv -f -i 1 -N c1 -F '%N %t %a %r %b' >/dev/null 2> >(stdbuf -o0 tr '\r' '\n' | while IFS= read -r l; do
        ms=${EPOCHREALTIME/./}
        printf '%.*s %s\n' 13 "$ms" "$l"
    done >c1.pv.log)

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
    local pat="${1?missing pattern}"
    shift
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
    local millis=$1
    if [[ -z $millis ]]; then read -r millis; fi
    local -i secs=$((millis / 1000))
    local -i sub=$((millis % 1000))
    local dp=
    dp=$(date -d "@$secs" '+%Y-%m-%d %H:%M:%S')
    echo "$dp.$(printf '%03d' $sub)"
}

secs() {
    local secs=$1
    if [[ -z "$secs" ]]; then read -r secs; fi
    if [[ -z $secs ]]; then
        echo "Usage: $0 <seconds>" >&2
        return 1
    else
        date -d@"$secs"
    fi
}

nanos() {
    local nanos=$1
    if [[ -z "$nanos" ]]; then read -r nanos; fi
    [[ -z $nanos ]] && {
        echo "Usage: $0 <nanos>" >&2
        return 1
    }
    date -d@$((nanos / (1000 * 1000 * 1000))).$((nanos % (1000 * 1000 * 1000)))
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

git_dir() {
    git rev-parse --git-dir
}

ignore_untracked() {
    local exclude_file
    local git_dir
    git_dir="$(git rev-parse --git-dir)"
    exclude_file="${git_dir}/info/exclude"
    (
        cd "$repo_root" || exit 1
        git status --porcelain | grep '^??' | sed 's/^?? //' | while read -r file; do
            echo "${file}" >>"$exclude_file"
        done

    )
    echo "added untracked files to $exclude_file" >&2
}

utils::urlencode() {
    local LC_ALL=C
    for ((i = 0; i < ${#1}; i++)); do
        : "${1:i:1}"
        case "$_" in
        [a-zA-Z0-9.~_-])
            printf '%s' "$_"
            ;;

        *)
            printf '%%%02X' "'$_"
            ;;
        esac
    done
    printf '\n'
}

utils::strip_first() {
    printf '%s\n' "${1/$2/}"
}

utils::strip_all() {
    printf '%s\n' "${1//$2/}"
}

utils::trim_quotes() {
    : "${1//\'/}"
    printf '%s\n' "${_//\"/}"
}

utils::upper() {
    printf '%s\n' "${1^^}"
}

utils::lower() {
    printf '%s\n' "${1,,}"
}

utils::split() {
    IFS=$'\n' read -d "" -ra arr <<<"${1//$2/$'\n'}"
    printf '%s\n' "${arr[@]}"
}

utils::trim() {
    : "${1#"${1%%[![:space:]]*}"}"
    : "${_%"${_##*[![:space:]]}"}"
    printf '%s\n' "$_"
}

utils::v4() {
    C="89ab"

    for ((N = 0; N < 16; ++N)); do
        B="$((RANDOM % 256))"

        case "$N" in
        6) printf '4%x' "$((B % 16))" ;;
        8) printf '%c%x' "${C:$RANDOM%${#C}:1}" "$((B % 16))" ;;

        3 | 5 | 7 | 9)
            printf '%02x-' "$B"
            ;;

        *)
            printf '%02x' "$B"
            ;;
        esac
    done
    printf '\n'
}
