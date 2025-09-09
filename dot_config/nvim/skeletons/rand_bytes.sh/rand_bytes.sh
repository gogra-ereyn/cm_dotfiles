#!/bin/bash

rand_bytes_ ()
{
  n_=$1

  chars_=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
  dev_rand_=/dev/urandom
  if test -r "$dev_rand_"; then
    # Note: 256-length($chars_) == 194; 3 copies of $chars_ is 186 + 8 = 194.
    dd ibs=$n_ count=1 if=$dev_rand_ 2>/dev/null \
      | LC_ALL=C tr -c $chars_ 01234567$chars_$chars_$chars_
    return
  fi

  (date; date +%N; free; who -a; w; ps auxww; ps -ef) 2>&1 | awk '
     BEGIN {
       n = '"$n_"'
       for (i = 0; i < 256; i++)
         ordinal[sprintf ("%c", i)] = i
     }
     {
       for (i = 1; i <= length; i++)
         a[ai++ % n] += ordinal[substr ($0, i, 1)]
     }
     END {
       chars = "'"$chars_"'"
       charslen = length (chars)
       for (i = 0; i < n; i++)
         printf "%s", substr (chars, a[i] % charslen + 1, 1)
       printf "\n"
     }
  '
}

