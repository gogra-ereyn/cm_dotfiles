#!/bin/bash
set -e

P2ABOVE="$1"
P2BELOW="$2"

fail=0
check() {
	local desc="$1" expected="$2" got="$3"
	if [ "$expected" != "$got" ]; then
		echo "FAIL: $desc: expected '$expected', got '$got'"
		fail=1
	else
		echo "ok: $desc"
	fi
}

# p2above - arg input
check "p2above 0"   "0" "$($P2ABOVE 0)"
check "p2above 1"   "1" "$($P2ABOVE 1)"
check "p2above 2"   "2" "$($P2ABOVE 2)"
check "p2above 3"   "4" "$($P2ABOVE 3)"
check "p2above 5"   "8" "$($P2ABOVE 5)"
check "p2above 100" "128" "$($P2ABOVE 100)"
check "p2above 128" "128" "$($P2ABOVE 128)"
check "p2above 1023" "1024" "$($P2ABOVE 1023)"
check "p2above 1024" "1024" "$($P2ABOVE 1024)"
check "p2above 1025" "2048" "$($P2ABOVE 1025)"

# p2above - stdin
check "p2above stdin" "256" "$(echo 200 | $P2ABOVE)"

# p2above - exponent
check "p2above -e 128" "7" "$($P2ABOVE -e 128)"
check "p2above -e 100" "7" "$($P2ABOVE -e 100)"
check "p2above -e 1"   "0" "$($P2ABOVE -e 1)"

# p2below - arg input
check "p2below 0"   "0" "$($P2BELOW 0)"
check "p2below 1"   "1" "$($P2BELOW 1)"
check "p2below 2"   "2" "$($P2BELOW 2)"
check "p2below 3"   "2" "$($P2BELOW 3)"
check "p2below 5"   "4" "$($P2BELOW 5)"
check "p2below 100" "64" "$($P2BELOW 100)"
check "p2below 128" "128" "$($P2BELOW 128)"
check "p2below 1023" "512" "$($P2BELOW 1023)"
check "p2below 1024" "1024" "$($P2BELOW 1024)"
check "p2below 1025" "1024" "$($P2BELOW 1025)"

# p2below - stdin
check "p2below stdin" "128" "$(echo 200 | $P2BELOW)"

# p2below - exponent
check "p2below -e 128" "7" "$($P2BELOW -e 128)"
check "p2below -e 100" "6" "$($P2BELOW -e 100)"
check "p2below -e 1"   "0" "$($P2BELOW -e 1)"

exit $fail
