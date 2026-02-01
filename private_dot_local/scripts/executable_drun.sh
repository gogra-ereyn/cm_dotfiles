#!/bin/bash

#
# drun - run commands in containers
#

set -euo pipefail

usage() {
    cat <<EOF
Usage: ${0##*/} [OPTIONS] [--] COMMAND [ARGS...]

Run commands inside a container with the current directory mounted.

Options:
    -h, --help              Show this help
    -i, --image IMAGE       Container image (default: \$DRUN_IMAGE or "oldstuff")
    -w, --workdir DIR       Working directory inside container (default: /work)
    -m, --mount SRC:DST     Additional mount (can be specified multiple times)
    -e, --env VAR=VAL       Environment variable (can be specified multiple times)
    -u, --user UID:GID      Run as UID:GID (default: current user)
    -r, --root              Run as root (uid 0)
    -n, --name NAME         Container name for persistent mode
    -p, --persistent        Use persistent container (faster repeated runs)
    -s, --shell             Start interactive shell instead of running command
        --no-tty            Don't allocate a TTY
        --network NET       Network mode (default: none)
        --rm-persistent     Remove persistent container and exit

Environment variables:
    DRUN_IMAGE              Default image
    DRUN_WORKDIR            Default workdir
    DRUN_USER               Default user mapping
    DRUN_NETWORK            Default network mode

Examples:
    ${0##*/} make clean
    ${0##*/} -i gcc:14 -- make -j8
    ${0##*/} -p -n myproject make test
    ${0##*/} -m ~/.ccache:/ccache -e CCACHE_DIR=/ccache make
    ${0##*/} --shell
EOF
    exit 0
}

main() {
    local image=
    local workdir=
    local user=
    local name=
    local network=
    local persistent=0
    local shell_mode=0
    local use_tty=1
    local rm_persistent=0
    local -a mounts=()
    local -a envvars=()
    local opts

    opts=$(getopt -o "hi:w:m:e:u:rn:ps" \
        --long "help,image:,workdir:,mount:,env:,user:,root,name:,persistent,shell,no-tty,network:,rm-persistent" \
        --name "${0##*/}" -- "$@")
    eval set -- "$opts"

    while (($#)); do
        case "$1" in
            -h|--help)          usage;;
            -i|--image)         image=$2; shift;;
            -w|--workdir)       workdir=$2; shift;;
            -m|--mount)         mounts+=("$2"); shift;;
            -e|--env)           envvars+=("$2"); shift;;
            -u|--user)          user=$2; shift;;
            -r|--root)          user="0:0";;
            -n|--name)          name=$2; shift;;
            -p|--persistent)    persistent=1;;
            -s|--shell)         shell_mode=1;;
            --no-tty)           use_tty=0;;
            --network)          network=$2; shift;;
            --rm-persistent)    rm_persistent=1;;
            --)                 shift; break;;
            *)                  return 1;;
        esac
        shift
    done

    : "${image:=${DRUN_IMAGE:-oldstuff}}"
    : "${workdir:=${DRUN_WORKDIR:-/work}}"
    : "${user:=${DRUN_USER:-$(id -u):$(id -g)}}"
    : "${network:=${DRUN_NETWORK:-}}"

    if ((rm_persistent)); then
        remove_persistent "$name"
        return 0
    fi

    if ((persistent)); then
        run_persistent "$name" "$@"
    else
        run_ephemeral "$@"
    fi
}

build_common_args() {
    local -n _args=$1; shift
    local -n _mounts=$1; shift
    local -n _envvars=$1; shift
    local workdir=$1; shift
    local user=$1; shift
    local network=$1; shift
    local use_tty=$1; shift

    _args=()

    if ((use_tty)) && [[ -t 0 ]]; then
        _args+=("-it")
    fi

    _args+=("-v" "$PWD:$workdir")
    _args+=("-w" "$workdir")

    if [[ -n "$user" ]]; then
        _args+=("-u" "$user")
    fi

    if [[ -n "$network" ]]; then
        _args+=("--network" "$network")
    fi

    local m
    for m in "${_mounts[@]}"; do
        _args+=("-v" "$m")
    done

    local e
    for e in "${_envvars[@]}"; do
        _args+=("-e" "$e")
    done
}

run_ephemeral() {
    local -a args
    local uid="${user%%:*}"
    local gid="${user##*:}"
    local tmp_files=""

    build_common_args args mounts envvars "$workdir" "$user" "$network" "$use_tty"

    if [[ -n "$uid" ]] && [[ "$uid" != "0" ]]; then
        tmp_files=$(setup_user_files args "$uid" "$gid")
    fi

    if ((shell_mode)) || (($# == 0)); then
        docker run --rm "${args[@]}" "$image" bash -l
    else
        docker run --rm "${args[@]}" "$image" bash -lc "$*"
    fi

    if [[ -n "$tmp_files" ]]; then
        rm -f "$tmp_files"
    fi
}

run_persistent() {
    local name=$1; shift

    if [[ -z "$name" ]]; then
        name="drun-${PWD##*/}"
        name="${name//[^a-zA-Z0-9_-]/_}"
    fi

    ensure_container_running "$name"

    local -a exec_args=()
    if ((use_tty)) && [[ -t 0 ]]; then
        exec_args+=("-it")
    fi

    if ((shell_mode)) || (($# == 0)); then
        docker exec "${exec_args[@]}" "$name" bash -l
    else
        docker exec "${exec_args[@]}" "$name" bash -lc "$*"
    fi
}

ensure_container_running() {
    local name=$1; shift

    if docker ps -q -f "name=^${name}$" | grep -q .; then
        return 0
    fi

    if docker ps -aq -f "name=^${name}$" | grep -q .; then
        docker start "$name" >/dev/null
        return 0
    fi

    local -a args
    build_common_args args mounts envvars "$workdir" "$user" "$network" 0

    docker run -d --name "$name" "${args[@]}" "$image" sleep infinity >/dev/null
}

remove_persistent() {
    local name=$1; shift

    if [[ -z "$name" ]]; then
        name="drun-${PWD##*/}"
        name="${name//[^a-zA-Z0-9_-]/_}"
    fi

    if docker ps -aq -f "name=^${name}$" | grep -q .; then
        docker rm -f "$name" >/dev/null
        echo "Removed container: $name"
    else
        echo "No container found: $name"
    fi
}

setup_user_files() {
    local -n _args=$1; shift
    local uid=$1; shift
    local gid=$1; shift

    if [[ -z "$uid" ]] || [[ "$uid" == "0" ]]; then
        return 0
    fi

    local tmp_passwd
    local tmp_group
    tmp_passwd=$(mktemp)
    tmp_group=$(mktemp)

    docker run --rm "$image" cat /etc/passwd > "$tmp_passwd" 2>/dev/null || true
    docker run --rm "$image" cat /etc/group > "$tmp_group" 2>/dev/null || true

    echo "hostuser:x:${uid}:${gid}::/home/hostuser:/bin/bash" >> "$tmp_passwd"
    echo "hostgroup:x:${gid}:" >> "$tmp_group"

    _args+=("-v" "${tmp_passwd}:/etc/passwd:ro")
    _args+=("-v" "${tmp_group}:/etc/group:ro")

    echo "$tmp_passwd $tmp_group"
}


main "$@"
