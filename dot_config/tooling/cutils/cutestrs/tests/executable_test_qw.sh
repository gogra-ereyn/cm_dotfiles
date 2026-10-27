#!/bin/bash

assert() {
    local actual="$1" expected="$2" msg="$3"
    if [ "$actual" = "$expected" ]; then
        printf '\033[32m✓\033[0m %s\n' "$msg" >&2
        return 0
    fi
    printf '\033[31m✗\033[0m %s\n  expected: %s\n  actual:   %s\n' "$msg" "$expected" "$actual" >&2
    return 1
}

testcase() {
    local input="$1" expected="$2" msg="${3:-$1}"
    local out_argv out_stdin

    out_argv="$("$BIN" "$input")"
    assert "$out_argv" "$expected" "[argv] $msg" || return 1

    out_stdin="$(printf '%s' "$input" | "$BIN")"
    assert "$out_stdin" "$expected" "[stdin] $msg" || return 1
}

main() {
    local sd
    sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    BIN="${1:-"${sd}/../build/bin/qw"}"

    local -i failed=0

    testcase 'hello'    "'hello'"    'simple word'        || (( ++failed ))
    testcase "hel'lo"   "'hel'\\''lo'"  'mid quote escaped'  || (( ++failed ))
    testcase "hello'"   "'hello'"    'trailing quote absorbed' || (( ++failed ))
    testcase "'hello"   "'hello'"    'leading quote absorbed'  || (( ++failed ))

    if (( failed > 0 )); then
        echo "FAILED: total $failed tests" >&2
    fi
    return 0
}

main "$@"
