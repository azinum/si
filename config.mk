# Makefile
# si - simple interpreter

PROGRAM_NAME=si

INCLUDE_DIR=include

SOURCE_DIR=src

BUILD_DIR=build

BUILD_DIR_DEBUG=$(BUILD_DIR)/debug

BUILD_DIR_RELEASE=$(BUILD_DIR)/release

BUILD_DIR_LIB=$(BUILD_DIR)/shared

INSTALL_TOP=/usr/local

INSTALL_BIN=$(INSTALL_TOP)/bin

LIBS=-lreadline -ldl -lm

LIBS_RELEASE=-lreadline -ldl -lm

FLAGS=$(SOURCE_DIR)/*.c -Iinclude -Wall -rdynamic

FLAGS_DEBUG=-g

FLAGS_LOCAL=-o $(BUILD_DIR_DEBUG)/$(PROGRAM_NAME) -O2 $(LIBS) -D TRACK_MEMORY -D USE_READLINE -D USE_COLORS

FLAGS_RELEASE=-o $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME) -O2 -Werror $(LIBS_RELEASE) -D NDEBUG -D USE_COLORS -D USE_READLINE -D TRACK_MEMORY

FLAGS_MINIMAL_BUILD=-o $(BUILD_DIR_DEBUG)/$(PROGRAM_NAME) -ldl -Os -D NO_JUMPTABLE

CC=gcc

LIB_NAME=lib$(PROGRAM_NAME)

LIB_PATH=/usr/local/lib

LIB_INCLUDE_PATH=/usr/local/include/$(PROGRAM_NAME)

FLAGS_LIB=-shared -fPIC -o $(BUILD_DIR_LIB)/$(LIB_NAME).so -D NDEBUG
