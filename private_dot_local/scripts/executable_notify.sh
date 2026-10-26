#!/usr/bin/env bash

die() {
    printf 'note-link: %s\n' "$*" >&2
    exit 1
}

cleanup() {
    local rc=$?
    trap - EXIT INT TERM

    if ((rc != 0 && rollback_active)); then
        if [[ -L $rollback_src && $(readlink -- "$rollback_src") == "$rollback_dst" ]]; then
            rm -- "$rollback_src" || printf 'note-link: could not remove link: %s\n' "$rollback_src" >&2
        fi
        if [[ ! -e $rollback_src && ! -L $rollback_src ]]; then
            mv -- "$rollback_dst" "$rollback_src" || printf 'note-link: recover file manually from: %s\n' "$rollback_dst" >&2
        else
            rm -f -- "$rollback_dst" || printf 'note-link: could not clean destination: %s\n' "$rollback_dst" >&2
        fi
    fi
    exit "$rc"
}

main() {
    set -euo pipefail

    local src_dir src notes repo rel key name subdir prefix hash
    local dest_dir file stem ext dst i

    rollback_src=''
    rollback_dst=''
    rollback_active=0
    trap cleanup EXIT
    trap 'exit 130' INT
    trap 'exit 143' TERM

    [[ $# -eq 1 && -n ${NOTES_DIR:-} ]] || die 'usage: NOTES_DIR=/path/to/notes note-link FILE'
    [[ -d $NOTES_DIR ]] || die 'NOTES_DIR must be an existing directory'
    [[ -f $1 && ! -L $1 ]] || die 'source must be a regular file, not a symlink'

    src_dir=$(realpath -e -- "$(dirname -- "$1")")
    src="$src_dir/$(basename -- "$1")"
    notes=$(realpath -e -- "$NOTES_DIR")
    [[ $src != "$notes"/* ]] || die 'file is already inside NOTES_DIR'

    repo=''
    if repo=$(git -C "$src_dir" rev-parse --show-toplevel 2>/dev/null); then
        repo=$(realpath -e -- "$repo")
        rel=${src#"$repo"/}
        [[ $rel != "$src" ]] || die 'could not determine project-relative path'
        [[ $notes != "$repo" && $notes != "$repo"/* ]] || die 'NOTES_DIR is inside the project'
        if git -C "$repo" ls-files --error-unmatch -- ":(literal)$rel" >/dev/null 2>&1; then
            die 'file is tracked by Git; untrack it first if you want to move it'
        fi
        key=$(git -C "$repo" config --get remote.origin.url || true)
        key=${key:-$repo}
        name=${repo##*/}
        subdir=$(dirname -- "$rel")
        prefix=projects
    else
        key=$src_dir
        name=${src_dir##*/}
        subdir=.
        prefix=loose
    fi

    name=${name//[^[:alnum:]_.-]/_}
    [[ -n $name && $name != . && $name != .. ]] || name=notes
    hash=$(printf '%s' "$key" | sha256sum)
    hash=${hash%% *}
    dest_dir="$notes/$prefix/$name-${hash:0:12}"
    [[ $subdir == . ]] || dest_dir+="/$subdir"
    mkdir -p -- "$dest_dir"
    dest_dir=$(realpath -e -- "$dest_dir")
    [[ $dest_dir == "$notes"/* ]] || die 'destination resolves outside NOTES_DIR'

    file=${src##*/}
    stem=${file%.*}
    ext=.${file##*.}
    if [[ -z $stem || $stem == "$file" ]]; then
        stem=$file
        ext=''
    fi

    for ((i = 0; i <= 9999; ++i)); do
        if ((i == 0)); then
            dst="$dest_dir/$file"
        else
            dst="$dest_dir/$stem-$i$ext"
        fi
        [[ -e $dst || -L $dst ]] && continue
        if (
            set -C
            : >"$dst"
        ) 2>/dev/null; then
            rollback_src=$src
            rollback_dst=$dst
            rollback_active=1
            break
        fi
        die "could not reserve destination: $dst"
    done
    ((rollback_active)) || die 'too many filename collisions'

    mv -- "$src" "$dst"
    ln -s -- "$dst" "$src"
    rollback_active=0
    trap - EXIT INT TERM
    printf '%s -> %s\n' "$src" "$dst"
}

main "$@"
