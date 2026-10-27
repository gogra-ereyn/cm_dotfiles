#!/bin/bash
set -euo pipefail

declare -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare -r bin="${1:-"${sd}/../build/bin/cpy"}"
declare -i failed=0

# decode the b64 payload from osc52 output, stripping the framing
osc52_decode() {
    local raw="$1"
    local payload

    payload="${raw:7}"
    payload="${payload%$'\x07'}"
    printf '%s' "$payload" | base64 -d
}

assert_roundtrip() {
    local input="$1"
    local expected="$2"
    local msg="$3"
    local raw decoded

    raw="$(printf '%s' "$input" | "$bin")"
    decoded="$(osc52_decode "$raw")"

    if [ "$decoded" = "$expected" ]; then
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    else
        printf '\033[0;31m✗\033[0m %s\n' "$msg"
        printf '  Expected: %q\n' "$expected"
        printf '  Decoded:  %q\n' "$decoded"
        (( ++failed ))
    fi
}

assert_file() {
    local file="$1"
    local expected="$2"
    local msg="$3"
    local raw decoded

    raw="$("$bin" "$file")"
    decoded="$(osc52_decode "$raw")"

    if [ "$decoded" = "$expected" ]; then
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    else
        printf '\033[0;31m✗\033[0m %s\n' "$msg"
        printf '  Expected: %q\n' "$expected"
        printf '  Decoded:  %q\n' "$decoded"
        (( ++failed ))
    fi
}

assert_exec() {
    local expected="$1"
    local msg="$2"
    shift 2
    local raw decoded

    raw="$("$bin" -e -- "$@")"
    decoded="$(osc52_decode "$raw")"

    if [ "$decoded" = "$expected" ]; then
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    else
        printf '\033[0;31m✗\033[0m %s\n' "$msg"
        printf '  Expected: %q\n' "$expected"
        printf '  Decoded:  %q\n' "$decoded"
        (( ++failed ))
    fi
}

# compare raw b64 payload directly (avoids bash $() stripping trailing newlines)
assert_b64() {
    local input="$1"
    local expected_b64="$2"
    local msg="$3"
    shift 3
    local raw actual_b64

    raw="$(printf '%s' "$input" | "$bin" "$@")"
    actual_b64="${raw:7}"
    actual_b64="${actual_b64%$'\x07'}"

    if [ "$actual_b64" = "$expected_b64" ]; then
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    else
        printf '\033[0;31m✗\033[0m %s\n' "$msg"
        printf '  Expected b64: %s\n' "$expected_b64"
        printf '  Actual b64:   %s\n' "$actual_b64"
        (( ++failed ))
    fi
}

assert_fails() {
    local msg="$1"
    local input="$2"

    if printf '%s' "$input" | "$bin" >/dev/null 2>&1; then
        printf '\033[0;31m✗\033[0m %s (expected failure, got success)\n' "$msg"
        (( ++failed ))
    else
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    fi
}

assert_exec_fails() {
    local msg="$1"
    shift

    if "$bin" -e -- "$@" >/dev/null 2>&1; then
        printf '\033[0;31m✗\033[0m %s (expected failure, got success)\n' "$msg"
        (( ++failed ))
    else
        printf '\033[0;32m✓\033[0m %s\n' "$msg"
    fi
}

main() {
    # basic stdin
    assert_roundtrip "hello" "hello" "simple word"
    assert_roundtrip "Hello World" "Hello World" "space in input"
    assert_roundtrip "a" "a" "single char"
    assert_roundtrip "ab" "ab" "two chars"
    assert_roundtrip "abc" "abc" "three chars (exact b64 boundary)"

    # trailing newlines should be trimmed
    assert_roundtrip $'hello\n' "hello" "trailing newline trimmed"
    assert_roundtrip $'hello\n\n\n' "hello" "multiple trailing newlines trimmed"
    assert_roundtrip $'hello\r\n' "hello" "trailing crlf trimmed"

    # interior newlines preserved
    assert_roundtrip $'line1\nline2' $'line1\nline2' "interior newline preserved"
    assert_roundtrip $'line1\nline2\n' $'line1\nline2' "interior newline preserved, trailing trimmed"

    # multi-line file via stdin
    local lorem_content lorem_trimmed
    lorem_content="$(<"${sd}/lorem.txt")"
    lorem_trimmed="${lorem_content%$'\n'}"
    assert_roundtrip "$lorem_content" "$lorem_trimmed" "lorem ipsum from file (stdin)"

    # binary-ish content
    assert_roundtrip "$(printf '\x01\x02\x03')" "$(printf '\x01\x02\x03')" "binary bytes"

    # -k flag: keep trailing newlines (compare b64 directly to avoid bash $() stripping)
    assert_b64 $'hello\n' "aGVsbG8K" "-k preserves trailing newline" -k
    assert_b64 $'hello\n\n\n' "aGVsbG8KCgo=" "-k preserves multiple trailing newlines" -k
    assert_b64 $'hello\r\n' "aGVsbG8NCg==" "-k preserves trailing crlf" -k

    # file mode
    assert_file "${sd}/lorem.txt" "$lorem_trimmed" "file arg reads file content"

    # exec mode
    assert_exec "hello" "exec printf" printf '%s' hello
    assert_exec "$lorem_trimmed" "exec cat file" cat "${sd}/lorem.txt"

    # exec mode failure propagation
    assert_exec_fails "exec failing command" false
    assert_exec_fails "exec missing command" __nonexistent_command_12345__

    # empty / whitespace-only should fail
    assert_fails "empty input rejected" ""
    assert_fails "whitespace-only input rejected" $'\n\n\n'

    if (( failed > 0 )); then
        printf '\n%d test(s) failed\n' "$failed"
        return 1
    fi
    printf '\nall tests passed\n'
    return 0
}

main "$@"
