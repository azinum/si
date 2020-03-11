// si.c
// simple interpreter
// author: lucas (azinum)

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>

#include "error.h"
#include "mem.h"
#include "vm.h"
#include "file.h"
#include "config.h"
#include "si.h"

#if defined(USE_READLINE)

#include <readline/readline.h>
#include <readline/history.h>

#define readinput(buffer, prompt) ((buffer = readline(prompt)) != NULL)
#define addhistory(buffer) (buffer[0] != '\0' ? add_history(buffer) : (void)0)
#define freebuffer(buffer) free(buffer)

#else

#define readinput(buffer, prompt) (printf(prompt), fgets(input, INPUT_MAX, stdin) != NULL)
#define addhistory(buffer)
#define freebuffer(buffer)

#endif

void signal_exit(int x) {
	printf("Enter ^D to exit\n");
}

int user_input(struct VM_state* vm) {
	assert(vm != NULL);
	char input[INPUT_MAX] = {0};
	char* buffer = input;
	int status = NO_ERR;
	int is_running = 1;
	while (is_running) {
		if (readinput(buffer, PROMPT)) {
			status = vm_exec(vm, buffer);
			if (status != NO_ERR)
				return status;
			addhistory(buffer);
			freebuffer(buffer);
		}
		else
			is_running = 0;
	}
	return NO_ERR;
}

int si_exec(int argc, char** argv) {
	signal(SIGINT, signal_exit);
	struct VM_state vm;
	vm_init(&vm);
	char* input = read_file("test/test.lang");
	if (input) {
		vm_exec(&vm, input);
		free(input);
	}
	print_memory_info();
	user_input(&vm);
	vm_state_free(&vm);
	return NO_ERR;
}