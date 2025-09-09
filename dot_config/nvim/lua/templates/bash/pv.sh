#!/bin/|ash


fifo_for_tar() {
    local tarfifo=${TARFIFO:-tarfifo}
    local target_dir="${TARGET_DIR?need compression target}"
    mkfifo "$tarfifo"
    tar -cvf - "$target_dir" | pv -s "$(du -sb /path/to/directory | awk '{print $1}')" > "$tarfifo" &
    gzip < "$tarfifo" > archive.tar.gz
    rm "$tarfifo"
}


generate_data() {
    echo "noop: example only"
}

datagen() {
    mkfifo fifo1
    mkfifo fifo2
    generate_data | pv -s 100M > fifo1 &
    process_data < fifo1 | pv -s 100M > fifo2 &
    cat fifo2 > final_output.dat
}

