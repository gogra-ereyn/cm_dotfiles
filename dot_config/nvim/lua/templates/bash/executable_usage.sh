#!/bin/bash

usage_str() {
cat << EOF
Usage: $0 [OPTIONS]

Options:

    -i, --interval <SECONDS>
        Duration to sleep between outputting lines
        [env: SLEEP_SECONDS=${SLEEP_SECONDS}]
        [default:0]

    -n, --num <COUNT>
        Total number of lines to output
        [env: COUNT=${COUNT}]
        [default: 100]

EOF
}

usage() {
    local -i rc=${1:-1}
    usage_str >&2
    exit $rc
}


main() {
    local ival="${SLEEP_SECONDS:-0}"
    local count="${COUNT:-20}"

    opts=$(getopt -o "hi:n:" --long "help,interval:,num:" --name "${0##*/}" -- "$@")
    eval set -- "$opts"

    while (($#)); do
        case "$1" in
            -h|--help)           usage;;
            -n|--num)            count=$2; shift;;
            -i|--inverval)       ival=$2; shift;;
            --)                  shift; break;;
            *)                   false
        esac
        shift
    done

    [[ -z $count ]] && usage 1

    for ((i=1; i<=count; i++)); do
        echo "line $i"
        sleep "$ival"
    done
}

main "$@"
