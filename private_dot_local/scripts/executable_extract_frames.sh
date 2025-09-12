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
    #local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

    local -i ival="${SLEEP_SECONDS:-0}"
    local -i count="${COUNT:-20}"

    local from=
    local until=

    opts=$(getopt -o "hi:c:f:u:" --long "help,interval:,count:" --name "${0##*/}" -- "$@")
    eval set -- "$opts"

    while (($#)); do
        case "$1" in
            -h|--help)           usage;;
            -f|--from)           from=$2; shift;;
            -u|--until)          until=$2; shift;;
            --)                  shift; break;;
            *)                   false
        esac
        shift
    done


    local input="${1?missing input}" ; shift
    local output="$1"

    if [[ -z "$output" ]]; then
        output="./$(basename "$input")_frames"
        mkdir -p "$output" &>/dev/null
    fi

    ffmpeg -i "$input" ${from:+"--from" "${from}"}${until:+" --until" "${until}"} -vf fps=30 -q:v 2 "${output}/%04d.png"
}

main "$@"


