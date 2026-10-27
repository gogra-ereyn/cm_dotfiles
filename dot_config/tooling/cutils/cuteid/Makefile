BUILD_DIR ?= build
CONFIG    ?= RelWithDebInfo
JOBS      ?= 4
ARGS      ?=
CMAKE     ?= cmake
CTEST     ?= ctest

.PHONY: all build debug release test install clean rebuild

all: build

build:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(ARGS)
	$(CMAKE) --build $(BUILD_DIR) -- -j $(JOBS)
	@ln -sf $(BUILD_DIR)/compile_commands.json .

debug:
	$(MAKE) build CONFIG=Debug

release:
	$(MAKE) build CONFIG=Release

test: debug
	$(CTEST) --output-on-failure --test-dir $(BUILD_DIR)

install: build
	$(CMAKE) --install $(BUILD_DIR)

clean:
	@echo "Removing $(BUILD_DIR)/"
	rm -rf $(BUILD_DIR)

rebuild: clean build
