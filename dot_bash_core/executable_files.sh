#!/bin/bash

lst() {
    local index=${1:-1}
    ls -t1 | tail -n "+$((index + 1))" | head -n 1
}

latest() {
    local dir="${1:-.}"
    local ext="${2}"
    echo "Dir=${dir}, ext=${ext}"
    if [ -n "$ext" ]; then
        find "$dir" -maxdepth 1 -type f -name "*.$ext" -printf "%P\n" | sort -n | tail -n 1
    else
        find "$dir" -maxdepth 1 -type f -printf "%P\n" | sort -n | tail -n 1
    fi
}

collect_img() {
    find . -type f \( -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o -iname "*.gif" -o -iname "*.bmp" -o -iname "*.tiff" -o -iname "*.webp" \) -exec cp {} "${1-/apps/collect/}" \;
}

find_images() {
    find "${1:-.}" -type f \( \
        -iname '*.jpg' -o \
        -iname '*.png' -o \
        -iname '*.jpeg' -o \
        -iname '*.webp' -o \
        -iname '*.3gp' \
        \)
}

find_videos() {
    find "${1:-.}" -type f \( \
        -iname '*.mp4' -o \
        -iname '*.mkv' -o \
        -iname '*.webm' -o \
        -iname '*.mov' -o \
        -iname '*.avi' -o \
        -iname '*.m4v' -o \
        -iname '*.wmv' -o \
        -iname '*.flv' -o \
        -iname '*.mpeg' -o \
        -iname '*.mpg' -o \
        -iname '*.3gp' \
        \)
}

shuf_play_vids() {
    find_videos "$@" | sort -u | shuf | mpv --playlist=-
}
