#!/bin/bash

main() {
    docker build -t "${1?missing container name}" .
    docker run --rm -it -v "$PWD":/work "$1" bash -lc '
      cmake -S /work -B /work/build -G Ninja -DCMAKE_BUILD_TYPE=Release &&
      cmake --build /work/build &&
      /work/build/containerrun
    '
}

main "$@"
