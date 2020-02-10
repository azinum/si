# Makefile - simple interpreter

CC=tcc

PROGRAM_NAME=si

SOURCE_DIR=src

BUILD_DIR=build

FLAGS=-o $(BUILD_DIR)/$(PROGRAM_NAME) $(SOURCE_DIR)/*.c -Iinclude -Wall

FLAGS_DEBUG=-g

FLAGS_LOCAL=-O2

FLAGS_RELEASE=-O2 -Werror

all: prepare local run

clean:
	rm build/*

prepare:
	@if [ ! -d $(BUILD_DIR) ]; then \
		mkdir $(BUILD_DIR); \
	fi \

local:
	$(CC) $(FLAGS) $(FLAGS_LOCAL)

install:
	$(CC) $(FLAGS) $(FLAGS_RELEASE)

build_debug:
	$(CC) $(FLAGS) $(FLAGS_DEBUG)

debug: build_debug run_debug

run:
	./$(BUILD_DIR)/$(PROGRAM_NAME)

run_debug:
	gdb ./$(BUILD_DIR)/$(PROGRAM_NAME)
