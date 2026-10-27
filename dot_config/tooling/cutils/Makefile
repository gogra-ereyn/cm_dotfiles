BUILD_DIR  := build
BUILD_TYPE ?= RelWithDebInfo

INSTALL_PREFIX ?= $(HOME)/.local
INSTALL_BINDIR  = $(INSTALL_PREFIX)/bin

.PHONY: all configure build test install clean

all: build

configure:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

install: build
ifdef BINS
	install -d $(INSTALL_BINDIR)
	$(foreach b,$(BINS),install -m 755 $(BUILD_DIR)/bin/$(b) $(INSTALL_BINDIR);)
else
	cmake --install $(BUILD_DIR)
endif

clean:
	rm -rf $(BUILD_DIR)
