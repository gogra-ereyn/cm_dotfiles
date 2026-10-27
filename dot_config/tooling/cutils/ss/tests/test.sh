#!/bin/bash


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
    local range="$1"; shift ;
    local input="$1"; shift ;
    local expected=$1 ; shift ;
    local msg="${1:-"$range $input"}"

    local -r bin="${SS_BIN}"

    local output=
    output="$("$bin" "$range" "$input")"

    assert_eq "$output" "$expected" "[argv]: $msg"
    local output=
    output="$("$bin" "$range" <<< "$input")"

    assert_eq "$output" "$expected" "[stdin] $msg"
}



main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    SS_BIN="${1:-"${sd}/../build/bin/ss"}"
    export SS_BIN

    local -i failed=0

    printf "\n\n"

    testcase '..3' 'hello' 'hel' || (( ++failed ))
    testcase '..=3' 'hello' 'hell' || (( ++failed ))

    testcase '1..3' 'hello' 'el' || (( ++failed ))
    testcase '1..=3' 'hello' 'ell' || (( ++failed ))

    testcase '1..' 'hello' 'ello' || (( ++failed ))

    testcase '0..' 'hello' 'hello' || (( ++failed ))
    testcase '..' 'hello' 'hello' || (( ++failed ))

    # negativo
    testcase '-3..'      'hello'   'llo' || (( ++failed ))
    testcase '..-2'      'hello'   'hel' || (( ++failed ))

    testcase '-1..'      'hello'   'o' || (( ++failed ))
    testcase '..-1'      'hello'   'hell' || (( ++failed ))

    testcase '-4..-1'    'hello'   'ell' || (( ++failed ))
    testcase '-3..=-1'   'hello'   'l' || (( ++failed ))
    testcase '-5..-3'    'hello'   'he' || (( ++failed ))

    (( failed > 0 )) && return 1
    return 0
}



main "$@"
