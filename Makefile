# Makefile - simple interpreter

include config.mk

all: prepare local run

clean:
	rm build/*

prepare:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR_DEBUG)
	@mkdir -p $(BUILD_DIR_RELEASE)

local:
	$(CC) $(FLAGS) $(FLAGS_LOCAL)

install:
	$(CC) $(FLAGS) $(FLAGS_RELEASE)
	chmod o+x $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME)
	cp $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME) $(INSTALL_BIN)/

build_debug:
	$(CC) $(FLAGS) $(FLAGS_DEBUG)

debug: build_debug run_debug

run:
	./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME)

run_debug:
	gdb ./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME)
