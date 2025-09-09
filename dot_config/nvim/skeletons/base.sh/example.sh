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
    local -i ival="${SLEEP_SECONDS:-0}"
    local -i count="${COUNT:-20}"

    opts=$(getopt -o "hi:c:" --long "help,interval:,count:" --name "${0##*/}" -- "$@")
    eval set -- "$opts"

    while (($#)); do
        case "$1" in
            -h|--help)           usage;;
            -c|--count)          count=$2; shift;;
            -i|--inverval)       ival=$2; shift;;
            --)                  shift; break;;
            *)                   false
        esac
        shift
    done

    [[ -z $count ]] && usage 1
    [[ -z $ival ]] && usage 1
}

main "$@"

