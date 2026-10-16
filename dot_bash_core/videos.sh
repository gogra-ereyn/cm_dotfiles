frames()
{
    local video="$1"
    local start="${2:-}"
    local end="${3:-}"
    local name dir
    local -a args=()

    if [[ -z "$video" || ! -f "$video" ]]; then
        printf 'usage: frames VIDEO [START [END]]\n' >&2
        return 1
    fi

    name="${video##*/}"
    name="${name%.*}"
    dir="$(dirname -- "$video")/frames_${name}"

    mkdir -p -- "$dir" || return

    args+=(-i "$video")

    [[ -n "$start" ]] && args+=(-ss "$start")
    [[ -n "$end" ]] && args+=(-to "$end")

    ffmpeg -hide_banner -loglevel error \
        "${args[@]}" \
        -fps_mode passthrough \
        -start_number 0 \
        -compression_level 1 \
        "$dir/%06d.png"
}

