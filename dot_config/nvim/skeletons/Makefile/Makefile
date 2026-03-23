BUILD_DIR ?= build
CONFIG    ?= RelWithDebInfo
JOBS      ?= $(shell echo $$(( $$(nproc) / 2 > 0 ? $$(nproc) / 2 : 1 )))
ARGS      ?=
FILTER    ?=
CMAKE     ?= cmake
CTEST     ?= ctest
BIN_PATH   = $(BUILD_DIR)/$(BIN)

CTEST_FLAGS := --output-on-failure --test-dir $(BUILD_DIR)
ifneq ($(FILTER),)
CTEST_FLAGS += -R $(FILTER)
endif

.PHONY: all build debug release test list-tests clean rebuild run run-%

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
	$(CTEST) $(CTEST_FLAGS)

list-tests: debug
	@$(CTEST) $(CTEST_FLAGS) -N

clean:
	@echo "Removing $(BUILD_DIR)/"
	rm -rf $(BUILD_DIR)

rebuild: clean build

run: build
	@if [ ! -x "$(BIN_PATH)" ]; then \
	    echo "❌  Executable '$(BIN_PATH)' not found. Did you add_executable($(BIN)) ?" >&2 ; \
	    exit 1 ; \
	fi
	@echo "▶️  Running $(BIN_PATH) $(ARGS)"
	@$(BIN_PATH) $(ARGS)


run-%: ; $(MAKE) run BIN=$* ARGS="$(ARGS)"

