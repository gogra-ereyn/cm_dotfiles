#!/bin/bash

do_something() {
    sleep 1
}

do_something_else() {
    sleep 2
}

main() {
    tar -cf - ./* | ( cd "${DIR}" && tar -xf - )

    return_codes=( "${PIPESTATUS[@]}" )

    if (( return_codes[0] != 0 )); then
        do_something
    fi

    if (( return_codes[1] != 0 )); then
        do_something_else
    fi
}

main "$@"
