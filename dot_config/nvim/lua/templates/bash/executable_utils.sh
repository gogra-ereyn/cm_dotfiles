#!/bin/bash

rwx_to_mode_()
{
  case $# in
    1) rwx=$1;;
    *) echo "$0: wrong number of arguments" 1>&2
      echo "Usage: $0 ls-style-mode-string" 1>&2
      return;;
  esac

  case $rwx in
    [ld-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxtT-]) ;;
    [ld-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxtT-][+.]) ;;
    *) echo "$0: invalid mode string: $rwx" 1>&2; return;;
  esac

 s='s/S/@/;s/s/x@/;s/@/s/'
  t='s/T/@/;s/t/x@/;s/@/t/'

  u=$(sed 's/^.\(...\).*/,u=\1/;s/-//g;s/^,u=$//;'$s <<< "$rwx")
  g=$(sed 's/^....\(...\).*/,g=\1/;s/-//g;s/^,g=$//;'$s <<< "$rwx")
  o=$(sed 's/^.......\(...\).*/,o=\1/;s/-//g;s/^,o=$//;'$s';'$t <<< "$rwx")
  echo "=$u$g$o"
}


retry_delay_()
{
  local test_func=$1
  local -i init_delay=$2
  local -i max_n_tries=$3
  shift 3 || return 1

  local -i attempt=1
  local -i num_sleeps=$attempt
  local time_fail
  while test $attempt -le $max_n_tries; do
    local delay=
    delay=$($AWK -v n=$num_sleeps -v s="$init_delay" \
                  'BEGIN { print s * n }')
    "$test_func" "$delay" "$@" && { time_fail=0; break; } || time_fail=1
    (( attempt++ ))
    (( num_sleeps*=2 ))
  done
  test "$time_fail" = 0
}

