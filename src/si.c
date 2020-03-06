// si.c
// simple interpreter
// author: lucas (azinum)

#include "error.h"
#include "vm.h"
#include "file.h"
#include "config.h"
#include "si.h"

int user_input(struct VM_state* vm) {
	assert(vm != NULL);
	char input[INPUT_MAX] = {0};
	int status = NO_ERR;
	int is_running = 1;
  while (is_running) {
    printf("> ");
    if (fgets(input, INPUT_MAX, stdin) != NULL) {
      status = vm_exec(vm, input);
      if (status != NO_ERR) return status;
    }
    else is_running = 0;
  }
	return NO_ERR;
}

int si_exec(int argc, char** argv) {
	(void)user_input;
	struct VM_state* vm = vm_state_new();
	if (!vm)
		return ERR;
	char* input = read_file("scripts/test.lang");
	if (input) {
		vm_exec(vm, input);
		free(input);
	}
	user_input(vm);
	vm_state_free(vm);
	return NO_ERR;
}