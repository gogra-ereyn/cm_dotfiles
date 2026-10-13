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

# via timg
imgs() {
    local dir="${1:-.}"
    local files=()

    shopt -s nullglob nocaseglob
    files=("$dir"/*.{jpg,jpeg,png,webp,gif,avif})
    shopt -u nullglob nocaseglob

    ((${#files[@]})) || return 1

    timg -pk --grid=4 --title='%b' --frames=1 "${files[@]}"
}

# examples img for pdf/mp4/gif
pdftimg() {
    timg -pk --frames=1 document.pdf
}

mp4timg() {
    timg -pk video.mp4
}

giftimg() {
    timg -pk --frames=1 animation.gif
}
