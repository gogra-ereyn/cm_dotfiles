#!/bin/bash
set -euo pipefail

bin="$1"
pass=0
fail=0

check() {
    local desc="$1" input="$2" expect_line="$3" expect_cursor="$4"
    shift 4
    local result line cursor

    result=$(printf '%s' "$input" | "$bin" "$@")
    line=$(echo "$result" | head -1)
    cursor=$(echo "$result" | tail -1)

    if [ "$line" = "$expect_line" ] && [ "$cursor" = "$expect_cursor" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL: $desc"
        echo "  expected: line='$expect_line' cursor=$expect_cursor"
        echo "  got:      line='$line' cursor=$cursor"
        fail=$((fail + 1))
    fi
}

# basic word surround with quote
check "surround first word with single quote" \
    "echo hello world" "'echo' hello world" "1" \
    -p 2 "'"

check "surround middle word with double quote" \
    "echo hello world" 'echo "hello" world' "6" \
    -p 7 '"'

# paired delimiters
check "surround word with parens" \
    "echo hello world" "(echo) hello world" "1" \
    -p 2 '('

check "surround word with brackets" \
    "echo hello world" "echo [hello] world" "6" \
    -p 7 '['

# WORD mode
check "WORD mode with punctuation" \
    "echo foo.bar world" "echo 'foo.bar' world" "6" \
    -W -p 6 "'"

# cursor at start of word
check "cursor at word start" \
    "echo hello" "echo 'hello'" "6" \
    -p 5 "'"

# cursor at end of line
check "cursor at end" \
    "echo hello" "echo 'hello'" "6" \
    -p 10 "'"

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
