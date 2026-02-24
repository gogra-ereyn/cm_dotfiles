#!/bin/bash

wait_tcp() {
    local host="${1?missing 1/host}"
    local -i port=${2?missing 2/port}
    local timeout=${3:-30}
    local end=$((SECONDS + timeout))
    while (( SECONDS < end )); do
        if (exec 3<>"/dev/tcp/$host/$port") >/dev/null 2>&1; then
            exec 3>&-
            exec 3<&-
            return 0
        fi
        sleep 0.2
    done
    return 1
}
