#!/bin/bash

declare -r isd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
source "${isd}/utils.sh"
declare bin="${isd}/../build/itoh"

testcase() {
    local -a flags=()
    while [[ $1 == -* ]]; do
        flags+=("$1")
        shift
    done
    local -i input="$1";shift;
    local expected=$1;shift;
    local msg="${1-$input}";shift;
    local output=
    output="$("$bin" "${flags[@]}" "$input")"
    assert_eq "$output" "$expected" "$msg" || return

    output="$("$bin" "${flags[@]}" <<< "$input")"
    assert_eq "$output" "$expected" "$msg: [input stdin]"
}


main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    "${sd}/../build.sh" || abort "build failed"

    testcase    1748639869 "00000000683a207d" "simple"
    testcase -z 1748639869 "683a207d" "minwidth"
    testcase -z 1748639869 "683a207d" "minwidth"
    testcase --bytes=4 1748639869 "683a207d" "width 32"

    testcase -z 1748639869 "683a207d" "minwidth"
    testcase --bytes=4 1748639869 "683a207d" "width 32"

    testcase -z 1748639869 "683a207d" "minwidth"
    testcase --esc='\x' -z  1748639869 '\x68\x3a\x20\x7d' "'\\x' seperator"
    testcase --esc="0x" --sep=' ' -z 1748639869 "0x68 0x3a 0x20 0x7d" "'0x' sep, ' ' esc"
}
main "$@"
