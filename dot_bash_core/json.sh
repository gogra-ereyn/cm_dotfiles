#!/bin/bash


jsondiff() {
    jq -S . "${1?missing left}" | diff -u - "${2?missing right}"
}


jsoneq() {
    jq -S . "${1?missing left}" == jq -S . "${2?missing right}"
}

jsoson_eq_bool() {
    jq -e --slurp '.[0] == .[1]' "${1?missing left}" "${1?missing right}" > /dev/null
}

json_eq_diff() {
    diff -u <(jq -S . a."${1?missing left}" ) <(jq -S . "${1?missing right}")
}
