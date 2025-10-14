#!/bin/bash

# sanity check run test instance
main() {
    export FDB_CLUSTER_FILE="$HOME/.config/foundationdb/fdb.cluster"
    mkdir -p ~/.local/share/foundationdb/data ~/.local/share/foundationdb/logs ~/.config/foundationdb
    echo 'test:test@127.0.0.1:4500' > ~/.config/foundationdb/fdb.cluster
    /usr/bin/fdbserver \ -C "$FDB_CLUSTER_FILE" -p auto:4500 &
    fdbcli --exec 'configure new single memory'
    fdbcli --exec 'status'
}

main "$@"
