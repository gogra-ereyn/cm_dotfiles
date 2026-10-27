#!/bin/bash

repo_root() {
    git rev-parse --show-toplevel 2>/dev/null || pwd
}

declare -r u_root="$(repo_root)"

assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"
    if [ "$actual" = "$expected" ]; then
        printf "\033[0;32m✓\033[0m %s\n" "$message"
        return 0
    else
        printf "\033[0;31m✗\033[0m %s\n" "$message"
        printf "Expected: %s\n" "$expected"
        printf "Actual:   %s\n'" "$actual"
        return 1
    fi
}

assert_eq() {
    assert "$1" "$2" "$3"
}

log() {
    local message="$1"
    local level="${2:-INFO}"
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo "[$timestamp] [$level] $message"
}

info() {
   log "$1" info
}

abort() {
   local msg="$1"
   local -i ec="${2-1}"
   log "$msg" fatal
   exit $ec
}

