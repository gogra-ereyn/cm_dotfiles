#!/bin/bash

set -e

COLOR_GREEN='\033[0;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_RED='\033[0;31m'
COLOR_RESET='\033[0m'

info() {
    echo -e "${COLOR_GREEN}==>${COLOR_RESET} $*"
}

warn() {
    echo -e "${COLOR_YELLOW}WARNING:${COLOR_RESET} $*"
}

error() {
    echo -e "${COLOR_RED}ERROR:${COLOR_RESET} $*"
    exit 1
}

check_prereqs() {
    info "Checking prerequisites..."

    if ! command -v podman &> /dev/null; then
        error "podman not found. Please install podman first."
    fi

    if ! command -v gcc &> /dev/null; then
        error "gcc not found. Please install gcc for building cexec."
    fi

    if ! command -v make &> /dev/null; then
        error "make not found. Please install make."
    fi

    info "All prerequisites found."
}

build_binary() {
    info "Building cexec binary..."
    make clean 2>/dev/null || true
    make
    info "Binary built successfully."
}

install_binary() {
    info "Installing cexec to ~/.local/bin..."
    make install

    if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        warn "~/.local/bin is not in your PATH"
        warn "Add this to your ~/.bashrc or ~/.zshrc:"
        echo ""
        echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
        echo ""
    else
        info "~/.local/bin is already in PATH"
    fi
}

build_images() {
    info "Building container images..."

    local targets=("dev" "c-dev" "cpp-dev" "cmake-dev" "auto-dev")
    local failed=0

    for target in "${targets[@]}"; do
        info "Building localhost/$target:latest..."
        if podman build --target "$target" -t "localhost/$target:latest" -f Dockerfile . &> /tmp/cexec-build-$target.log; then
            info "  ✓ $target built successfully"
        else
            warn "  ✗ Failed to build $target (see /tmp/cexec-build-$target.log)"
            failed=$((failed + 1))
        fi
    done

    if [ $failed -eq 0 ]; then
        info "All images built successfully!"
    else
        warn "$failed image(s) failed to build. Check logs in /tmp/cexec-build-*.log"
    fi
}

setup_ccache() {
    info "Setting up ccache..."

    if [ ! -d "$HOME/.ccache" ]; then
        mkdir -p "$HOME/.ccache"
        info "Created ~/.ccache directory"
    else
        info "~/.ccache already exists"
    fi

    if command -v ccache &> /dev/null; then
        ccache --set-config=max_size=5G 2>/dev/null || true
        ccache --set-config=compression=true 2>/dev/null || true
        info "Configured ccache (5GB max, compression enabled)"
    else
        warn "ccache not found on host. Install it for faster builds:"
        warn "  sudo dnf install ccache"
    fi
}

create_example_project() {
    local example_dir="$HOME/cexec-example"

    if [ -d "$example_dir" ]; then
        warn "Example project already exists at $example_dir"
        return
    fi

    info "Creating example project at $example_dir..."

    mkdir -p "$example_dir"
    cd "$example_dir"

    cat > main.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    printf("Hello from cexec example!\n");
    printf("Arguments:\n");

    for (i = 0; i < argc; i++) {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }

    return EXIT_SUCCESS;
}
EOF

    cat > Makefile << 'EOF'
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = example

.PHONY: all clean run

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) arg1 arg2 arg3
EOF

    cat > README.md << 'EOF'
# cexec Example Project

This is a simple example project to test cexec.

## Usage

```bash
# Build the project
cexec make

# Run the program
cexec ./example

# Or build and run
cexec make run

# Clean
cexec make clean
```

## Testing

```bash
# Verbose mode to see what cexec is doing
cexec -v make

# Use ephemeral container
cexec -e make

# Use specific image
cexec -i localhost/c-dev:latest make
```
EOF

    info "Example project created at $example_dir"
    info "Try it:"
    echo ""
    echo "    cd $example_dir"
    echo "    cexec make"
    echo "    cexec ./example"
    echo ""
}

show_usage() {
    cat << EOF
cexec setup script

Usage: $0 [OPTIONS]

Options:
    --binary-only       Only build and install the cexec binary
    --images-only       Only build container images
    --no-examples       Skip creating example project
    --help              Show this help

Default: Does everything (build binary, install, build images, setup ccache, create example)
EOF
}

main() {
    local binary_only=0
    local images_only=0
    local skip_examples=0

    while [[ $# -gt 0 ]]; do
        case $1 in
            --binary-only)
                binary_only=1
                shift
                ;;
            --images-only)
                images_only=1
                shift
                ;;
            --no-examples)
                skip_examples=1
                shift
                ;;
            --help)
                show_usage
                exit 0
                ;;
            *)
                error "Unknown option: $1"
                ;;
        esac
    done

    echo ""
    info "cexec setup starting..."
    echo ""

    check_prereqs

    if [ $images_only -eq 0 ]; then
        build_binary
        install_binary
        setup_ccache
    fi

    if [ $binary_only -eq 0 ]; then
        build_images
    fi

    if [ $skip_examples -eq 0 ] && [ $binary_only -eq 0 ]; then
        create_example_project
    fi

    echo ""
    info "Setup complete!"
    echo ""
    info "Next steps:"
    echo "  1. Ensure ~/.local/bin is in your PATH"
    echo "  2. Try: cexec --help"
    echo "  3. Test with the example project at ~/cexec-example"
    echo ""
    info "For Zellij integration, add aliases to your shell rc:"
    echo "  alias dmake='cexec make'"
    echo "  alias dgcc='cexec gcc'"
    echo "  alias dgdb='cexec gdb'"
    echo ""
}

main "$@"
