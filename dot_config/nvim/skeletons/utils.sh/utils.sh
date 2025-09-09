#!/bin/bash

utils::repo_root() {
    git rev-parse --show-toplevel 2>/dev/null || pwd
}

declare -r utils_root="$(utils::repo_root)"

utils::assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"
    if [ "$actual" = "$expected" ]; then
        echo -e "\033[0;32m✓\033[0m $message"
        return 0
    else
        echo -e "\033[0;31m✗\033[0m $message"
        echo -e "Expected: '$expected'"
        echo -e "Actual:   '$actual'"
        return 1
    fi
}

utils::assert_eq() {
    assert "$1" "$2" "$3"
}

utils::log() {
    local message="$1"
    local level="${2:-INFO}"
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo "[$timestamp] [$level] $message"
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

