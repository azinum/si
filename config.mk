# si - simple interpreter

PROGRAM_NAME=si

INCLUDE_DIR=include

SOURCE_DIR=src

BUILD_DIR=build

BUILD_DIR_DEBUG=$(BUILD_DIR)/debug

BUILD_DIR_RELEASE=$(BUILD_DIR)/release

INSTALL_TOP=/usr/local

INSTALL_BIN=$(INSTALL_TOP)/bin

LIBS=-lreadline

LIBS_RELEASE=

FLAGS=$(SOURCE_DIR)/*.c -Iinclude -Wall

FLAGS_DEBUG=-g

FLAGS_LOCAL=-o $(BUILD_DIR_DEBUG)/$(PROGRAM_NAME) -O2 $(LIBS) -D TRACK_MEMORY -D USE_READLINE -D USE_JUMPTABLE -D USE_COLORS

FLAGS_RELEASE=-o $(BUILD_DIR_RELEASE)/$(PROGRAM_NAME) -O2 -Werror $(LIBS_RELEASE) -D NDEBUG -D USE_JUMPTABLE  -D USE_COLORS

FLAGS_MINIMAL_BUILD=-o $(BUILD_DIR_DEBUG)/$(PROGRAM_NAME) -Os

CC=gcc
