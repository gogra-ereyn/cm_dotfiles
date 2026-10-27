#!/bin/bash

declare -r isd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare bin="${1:-"${isd}/../build/bin/cuteid"}"
source "${isd}/utils.sh"

PASS=0
FAIL=0

check() {
	local desc="$1" expected="$2" actual="$3"
	if [ "$expected" = "$actual" ]; then
		PASS=$((PASS+1))
		printf "\033[0;32m✓\033[0m %s\n" "$desc"
	else
		FAIL=$((FAIL+1))
		printf "\033[0;31m✗\033[0m %s\n" "$desc" >&2
		printf "  expected: %s\n" "$expected" >&2
		printf "  actual:   %s\n" "$actual" >&2
	fi
}

check_line() {
	local desc="$1" expected="$2" output="$3"
	local actual
	actual="$(echo "$output" | grep "^${expected%%=*}=")"
	check "$desc" "$expected" "$actual"
}

# ---- encode: known vectors ---- #

# epoch, minimal fields
uuid=$("$bin" encode 1 1767225600 42 7)
check "encode: basic alert" \
	"00000000-0004-8000-8000-002a00000007" "$uuid"

# epoch, process (no alert_handle)
uuid_proc=$("$bin" encode 1 1767225600 42)
check "encode: process (alert_handle=0)" \
	"00000000-0004-8000-8000-002a00000000" "$uuid_proc"

# non-zero start_time, server_id=15, larger handles
uuid2=$("$bin" encode 15 1767225601 1000000 500000)
check "encode: non-trivial fields" \
	"0000003a-983c-8000-8000-f42400079120" "$uuid2"

# ---- decode: roundtrip ---- #

out=$("$bin" decode "$uuid")
check_line "decode: server_id"       "server_id=1"       "$out"
check_line "decode: start_time"      "start_time=1767225600" "$out"
check_line "decode: process_handle"  "process_handle=42"  "$out"
check_line "decode: alert_handle"    "alert_handle=7"     "$out"
check_line "decode: type=alert"      "type=alert"         "$out"

out2=$("$bin" decode "$uuid_proc")
check_line "decode: alert_handle=0"  "alert_handle=0"     "$out2"
check_line "decode: type=process"    "type=process"       "$out2"

# ---- stdin input ---- #

out3=$(echo "$uuid" | "$bin" decode)
check_line "stdin: decode works" "process_handle=42" "$out3"

# ---- info ---- #

info_out=$("$bin" info "$uuid")
check_line "info: version=8"    "version=8 (custom)" "$info_out"
check_line "info: variant"      "variant=1 (rfc9562)" "$info_out"
check_line "info: server_id"    "server_id=1"         "$info_out"

# info on a v4 uuid (random, well-known test vector)
info_v4=$("$bin" info "550e8400-e29b-41d4-a716-446655440000")
check_line "info v4: version" "version=4 (random)" "$info_v4"

# ---- python interop ---- #

if command -v python3 &>/dev/null; then
	python3 -c "
import uuid, sys
u = uuid.UUID('$uuid')
assert u.version == 8, f'version: expected 8, got {u.version}'
assert u.variant == uuid.RFC_4122, f'variant: expected RFC_4122, got {u.variant}'
# roundtrip through python's formatter
assert str(u) == '$uuid', f'format: expected $uuid, got {str(u)}'
" && check "python: uuid parses + roundtrips" "ok" "ok" \
  || check "python: uuid parses + roundtrips" "ok" "FAILED"

	# compact (no hyphens) should also parse
	compact=$(echo "$uuid" | tr -d '-')
	python3 -c "
import uuid
u = uuid.UUID('$compact')
assert u.version == 8
" && check "python: compact format parses" "ok" "ok" \
  || check "python: compact format parses" "ok" "FAILED"
else
	printf "  (python3 not found, skipping interop tests)\n"
fi

# ---- summary ---- #

echo "---"
echo "$PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
