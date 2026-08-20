#!/bin/bash

main() {
    local pfx="${1?missing prefix}"
    local b="${2:-origin/master}"
    git rebase "${b}" --exec "msg=\$(git log -1 --format=%B); git commit --amend -m \"[${pfx}] \$msg\""
}

main "$@"
