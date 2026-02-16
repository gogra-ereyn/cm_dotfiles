#!/bin/bash

main() {

opts=$(getopt -o "hi:" --long "help,id:" --name "${0##*/}" -- "$@")
    eval set -- "$opts"
    local id=
    local rotate=
    while (($#)); do
        case "$1" in
            -h|--help)           usage;;
            -i|--id)             id=$2; shift;;
            --)                  shift; break;;
            *)                   false
        esac
        shift
    done

    : "${id:=DP-2}"
    : "${rotate:=${1:-normal}}"

    xrandr --output "$id" --rotate "$rotate"
}

main "$@"
