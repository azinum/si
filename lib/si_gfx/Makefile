# Makefile

LIB_NAME=si_gfx

CC=gcc

SRC=si_gfx.c

FLAGS=-I/usr/local/include/si -Wall -shared -fPIC -o lib${LIB_NAME}.so

LIBS=-lSDL2 -lsi

all:
	${CC} ${SRC} ${FLAGS} ${LIBS}
	./main.si

