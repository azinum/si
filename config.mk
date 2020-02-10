# si - simple interpreter

PROGRAM_NAME=si

SOURCE_DIR=src

BUILD_DIR=build

FLAGS=-o $(BUILD_DIR)/$(PROGRAM_NAME) $(SOURCE_DIR)/*.c -Iinclude -Wall

FLAGS_DEBUG=-g

FLAGS_LOCAL=-O2

FLAGS_RELEASE=-O2 -Werror

# Compiler and linker
CC=tcc
