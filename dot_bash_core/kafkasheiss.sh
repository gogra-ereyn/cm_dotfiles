
ckcat() {
    exec kcat -f '{"timestamp":%T,"offset":%o, "partition:%p, "payload":%s"}\n' "$@"
}

