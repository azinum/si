// si.c
// simple interpreter
// author: lucas (azinum)

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <argp.h>

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

static char args_doc[] = "";
static char doc[] = "si - simple interpreter";

static struct argp_option options[] = {
  {"interactive",   'i',  0,        0,    "run interactive mode after executing script"},
  {"no-warn",       'w',  0,        0,    "disable warnings"},
  {"bytecode-out",  'o',  0,        0,    "output byte code to file"},
  { 0 },
};

struct Args {
  char* input_file;
  int show_warnings;
  int interactive_mode;
  int bytecode_out;
};

static error_t parse_option(int key, char* arg, struct argp_state* state) {
  struct Args* arguments = state->input;
  int arg_count = state->argc - state->next;
  char** args = state->argv + state->next;
  (void)arg_count;
  (void)args;
  switch (key) {
    case 'i':
      arguments->interactive_mode = 1;
      break;
    case 'w':
      arguments->show_warnings = 0;
      break;
    case 'o':
      arguments->bytecode_out = 1;
      break;
    default:
      if (arg)
        arguments->input_file = arg;
      break;
  }
  return 0;
}

void signal_exit(int x) {
  printf("Enter ^D to exit\n");
}

int user_input(struct VM_state* vm) {
  assert(vm != NULL);
  char input[INPUT_MAX] = {0};
  char* buffer = input;
  int status = NO_ERR;
  unsigned char is_running = 1;
  char filename[] = "stdin";
  while (is_running) {
    if (readinput(buffer, PROMPT)) {
      status = vm_exec(vm, filename, buffer);
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
  (void)signal_exit;  // signal(SIGINT, signal_exit);
  struct argp argp = {options, parse_option, args_doc, doc};
  struct Args arguments = {
    .input_file = NULL,
    .show_warnings = 1,
    .interactive_mode = 0,
    .bytecode_out = 0
  };
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  error_init(arguments.show_warnings);
  struct VM_state vm;
  vm_init(&vm);
  if (arguments.input_file) {
    char* input = read_file(arguments.input_file);
    if (input) {
      vm_exec(&vm, arguments.input_file, input);
      if (arguments.bytecode_out) {
        char out_filename[INPUT_MAX];
        sprintf(out_filename, "%s.out", arguments.input_file);
        vm_disasm(&vm, out_filename);
      }
      free(input);
    }
  }
  if (arguments.interactive_mode || argc <= 1)
    user_input(&vm);
  vm_state_free(&vm);
  return NO_ERR;
}