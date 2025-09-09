#!/bin/bash

take() {
    local dir="${1?Missing dir name}"
    mkdir "$dir" && cd "$dir"
}

