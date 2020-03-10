// main.c
// tabsize: 2

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>

#include "si.h"

int main(int argc, char** argv) {
	return si_exec(argc, argv);
}