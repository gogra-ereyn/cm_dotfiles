frames() {
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
        "$dir/${name}_frame_%06d.png"
}

chooseimgs() {
    local src dest file
    src=${1:?usage: chooseimgs <source-dir> [dest-dir]}
    dest=${2:-"$src/chosen"}

    [[ -d "$src" ]] || {
        printf 'chooseimgs: not a directory: %s\n' "$src" >&2
        return 1
    }

    mkdir -p -- "$dest"

    while IFS= read -r -d '' file; do
        cp --backup=numbered -- "$file" "$dest"/
    done < <(
        find "$src" -maxdepth 1 -type f \
            \( -iname '*.png' -o -iname '*.jpg' -o -iname '*.jpeg' \
            -o -iname '*.webp' -o -iname '*.gif' -o -iname '*.bmp' \
            -o -iname '*.tif' -o -iname '*.tiff' -o -iname '*.avif' \
            -o -iname '*.heic' \) \
            -print0 |
            sort -z |
            nsxiv -0 -i -t -o
    )
}

choosepipe()
{
    local dest="${1:-./chosen}"
    local file
    local count=0

    mkdir -p -- "$dest" || return

    while IFS= read -r -d '' file; do
        cp --backup=numbered -- "$file" "$dest/" || return
        ((count+=1))
    done < <(nsxiv -0 -ito)

    printf 'copied %d images to %s\n' "$count" "$dest"
}
