#!/bin/bash

declare -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

declare bin="${1:-${sd}/../build/mpv}"

assert() {
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

assert_eq() {
    assert "$1" "$2" "$3"
}


testcase() {
    local input="$1"; shift ;
    local expected=$1 ; shift ;
    local msg="${1:-$input}"
    local output=
    output="$("$bin" "$input")"
    assert_eq "$output" "$expected" "$msg"
}

testcase_literal() {
    local input="${1?missing string bruv}"
    local out=
    out="$("$bin" < <(printf "%s" "$input"))"
    assert_eq ${#input} ${#out} "string lengths"
    assert_eq "$input" "$out"
}

main() {
    local -i failed=0
    testcase_literal "hello how r u" || ((++failed))
    info "Finished. Total failed=$failed"
    (( failed > 0 )) && return 1
    return 0
}



main "$@"
