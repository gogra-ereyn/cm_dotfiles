#!/bin/bash

declare -r isd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare bin="${1:-"${isd}/../build/bin/hex"}"
source "${isd}/utils.sh"

abort() {
   local msg="$1"
   local -i ec="${2-1}"
   echo "$msg" >&2
   exit $ec
}

testcase() {
    local -a flags=()
    while [[ $1 == -* ]]; do
        flags+=("$1")
        shift
    done

    local input="$1";shift;
    local expected="$1";shift;
    local msg="${1-$input}";shift;
    local output=
    output="$("$bin" "${flags[@]}" "$input")"
    assert_eq "$output" "$expected" "$msg"
}


decode_testcase() {
    local -a flags=()
    while [[ $1 == -* ]]; do
        flags+=("$1")
        shift
    done

    local encoded="$1";  shift
    local expected="$1"; shift
    local msg="${1:-$encoded}"

    local output=
    output="$("$bin" -d "${flags[@]}" "$encoded")"
    assert_eq "$output" "$expected" "[decode]: $msg"
}

main() {
    testcase "hello" "68656c6c6f"
    testcase "hello" "68656c6c6f"
    testcase $'hello\n' "68656c6c6f" "trailing newline"
    testcase $'hello\n\n\n' "68656c6c6f" "multiple trailing newline"

    testcase -u "hello" "68656C6C6F" "uppercase"


    # deocode
    decode_testcase "68656c6c6f" "hello" "decode simple"

    # whitespaceflag
    testcase -w "hello" "68656c6c6f"
    testcase -w $'hello\n' "68656c6c6f0a" "trailing newline"
    testcase -w $'hello\n\n\n' "68656c6c6f0a0a0a" "multiple trailing newline"

    #decode_testcase -w "68656c6c6f0a0a0a" $'hello\n\n\n' "decode keeps WS"

    # whitespaceflag
    testcase -w "hello" "68656c6c6f"
    testcase -w $'hello\n' "68656c6c6f0a" "trailing newline"
    testcase -w $'hello\n\n\n' "68656c6c6f0a0a0a" "multiple trailing newline"

    # simple sep
    testcase --seperator=':' "hello"       "68:65:6c:6c:6f"                 "':' sep"
    testcase --seperator=':' $'hello\n'  "68:65:6c:6c:6f"                   "':' sep trim"
    testcase -w --seperator=':' $'hello\n' "68:65:6c:6c:6f:0a"              "':' sep keep"

    # common seps
    testcase --seperator=' ' --esc='0x' "hello" "0x68 0x65 0x6c 0x6c 0x6f"  'sep=" ", esc="0x"'
    testcase -u --seperator=' ' --esc='0x' "hello" "0x68 0x65 0x6C 0x6C 0x6F"  'sep=" ", esc="0x", uppercase'

    decode_testcase --esc='0x' "0x680x650x6c0x6c0x6f" "hello" "'0x' esc"
    decode_testcase --esc='\x' '\x68\x65\x6c\x6c\x6f' "hello" "'\\x' esc"
}


main "$@"
