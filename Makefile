# Makefile - simple interpreter

include config.mk

all: prepare local run

clean:
	rm -drf build/*

prepare:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR_DEBUG)
	@mkdir -p $(BUILD_DIR_RELEASE)
	@mkdir -p $(BUILD_DIR_LIB)

local:
	$(CC) $(FLAGS) $(FLAGS_LOCAL)

install:
	$(CC) $(FLAGS) $(FLAGS_RELEASE)
	chmod o+x $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME)
	cp $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME) $(INSTALL_BIN)/$(PROGRAM_NAME)

shared:
	$(CC) $(FLAGS) $(FLAGS_LIB)
	chmod o+x $(BUILD_DIR_LIB)/$(LIB_NAME).so
	cp $(BUILD_DIR_LIB)/$(LIB_NAME).so $(LIB_PATH)/

uninstall:
	rm -f $(INSTALL_BIN)/$(PROGRAM_NAME)

build_debug:
	$(CC) $(FLAGS) $(FLAGS_LOCAL) $(FLAGS_DEBUG)

build_minimal:
	$(CC) $(FLAGS) $(FLAGS_MINIMAL_BUILD)

minimal: build_minimal run

debug: build_debug run_debug

run:
	./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME) -i -o test/test.si

run_debug:
	gdb ./$(BUILD_DIR_DEBUG)/$(PROGRAM_NAME)
