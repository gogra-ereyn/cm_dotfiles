#!/bin/bash

utils::wait_tcp() {
    local host="${1?missing 1/host}"
    local -i port=${2?missing 2/port}
    local timeout=${3:-30}
    local end=$((SECONDS + timeout))
    while (( SECONDS < end )); do
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
    [[ -z ${1:-} ]] && { echo "read_sleep: missing operand" >&2; return 1; }
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
    (($#<2)) && { echo "usage: log LEVEL message..." >&2; return 1; }
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
    : "${1/\#}"
    ((r=16#${_:0:2},g=16#${_:2:2},b=16#${_:4:2}))
    printf '%s\n' "$r $g $b"
}

utils::info() {
   utils::log info "$1"
}

utils::abort() {
   local msg="$1"
   local -i ec=${2-1}
   utils::log fatal "$msg"
   exit $ec
}

