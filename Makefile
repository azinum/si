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

uninstall:
	[ -f $(INSTALL_BIN)/$(PROGRAM_NAME) ] && rm $(INSTALL_BIN)/$(PROGRAM_NAME)

build_debug:
	$(CC) $(FLAGS) $(FLAGS_LOCAL) $(FLAGS_DEBUG)

build_minimal:
	$(CC) $(FLAGS) $(FLAGS_MINIMAL_BUILD)

minimal: build_minimal run

debug: build_debug run_debug

run:
	./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME)

run_debug:
	gdb ./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME)
