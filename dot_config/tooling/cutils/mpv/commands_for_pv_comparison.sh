#!/bin/bash

# for illustration. to cleanup & make appropariately configurble if use

with_dd() {
    dd if=/dev/zero bs=1M count=100 status=none | ./mvp > /dev/null
    dd if=/dev/zero bs=1M count=100 status=none | pv > /dev/null
}


# more realism - random data - but slowa
with_rand_dd() {
    dd if=/dev/urandom bs=1M count=100 status=none | ./mvp > /dev/null
    dd if=/dev/urandom bs=1M count=100 status=none | pv > /dev/null
}

rate_limited() {
    dd if=/dev/zero bs=1M count=100 status=none | pv -L 1M | ./mvp > /dev/null
}

slower_source() {
    yes "$(printf '%1024s')" | head -n 10000 | ./mvp > /dev/null
}

split_same_stream() {
    dd if=/dev/zero bs=1M count=100 status=none | tee >(pv > /tmp/pv.out) | ./mvp > /tmp/mvp.out
}


maybe_most_striaghtforward() {
dd if=/dev/zero bs=1M count=100 iflag=fullblock status=none | ./mvp > /dev/null
}
