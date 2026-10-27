#!/bin/bash

declare -r isd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare bin="${isd}/../build/htoi"

abort() {
   local msg="$1"
   local -i ec="${2-1}"
   echo "$msg" >&2
   exit $ec
}

assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"

    if [[ "$actual" == "$expected" ]]; then
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
    local -a flags=()
    while [[ $1 == -* ]]; do
        flags+=("$1")
        shift
    done

    local input="$1";shift;
    local -i expected=$1;shift;
    local msg="${1-$input}";shift;
    local -i output=
    output="$("$bin" "${flags[@]}" "$input")"
    assert_eq "$output" "$expected" "$msg"
}


main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    "${sd}/../build.sh" || abort "build failed"


    testcase "683a207d" 1748639869 simple
    testcase "00000000683a207d" 1748639869 "leading 0s"

    testcase --seperator="\x" "683a207d" 1748639869

    testcase --seperator="\x" "683a207d" 1748639869

}

main "$@"
