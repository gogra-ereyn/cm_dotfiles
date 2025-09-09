

main() {

    local sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    local fifo_in="${sd}/fifo_in"
    local fifo_out="${sd}fifo_out"

    cleanup() {
        rm -f "$fifo_in" "$fifo_out"
    }

    trap 'rm "$fifo_in" "$fifo_out" &>/dev/null' EXIT

    mkfifo "$fifo_in" "$fifo_out"

    "$@" < "$fifo_in" > "$fifo_out" &

    cmd_pid=$!

    pv "$fifo_out" > /dev/null &
    pv_pid=$!
    dd if=/dev/zero bs=1M status=none > "$fifo_in" &
    data_pid=$!

    wait "$cmd_pid"
    kill "$data_pid" "$pv_pid" 2>/dev/null
    wait

}
main "$@"
