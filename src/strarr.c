// strarr.c

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "mem.h"
#include "str.h"
#include "strarr.h"

void strarr_init(struct Str_arr* arr) {
  assert(arr != NULL);
  arr->strings = NULL;
  arr->count = 0;
}

int strarr_append(struct Str_arr* arr, const char* str) {
  assert(arr != NULL);
  int str_length = strlen(str);
  char* str_copy = string_new_copy(str, str_length);
  printf("str_copy: %s\n", str_copy);
  if (!(&arr->strings[0])) {
    arr->strings = mmalloc(sizeof(char*));
    if (!arr->strings)
      return -1;
    arr->strings[arr->count++] = str_copy;
    return 0;
  }

  char** temp = mrealloc(arr->strings, sizeof(char*) * arr->count, sizeof(char*) * (arr->count + 1));
  if (!temp)
    return -1;
  arr->strings = temp;
  arr->strings[arr->count++] = str_copy;
  return 0;
}

char* strarr_top(struct Str_arr* arr) {
  assert(arr != NULL);
  if (arr->count > 0)
    return arr->strings[arr->count - 1];
  return NULL;
}

char** strarr_top_addr(struct Str_arr* arr) {
  assert(arr != NULL);
  if (arr->count > 0)
    return &arr->strings[arr->count - 1];
  return arr->strings;
}

void strarr_free(struct Str_arr* arr) {
  assert(arr != NULL);
  if (!arr->strings)
    return;
  for (int i = 0; i < arr->count; i++) {
    int length = strlen(arr->strings[i]);
    string_nfree(arr->strings[i], length);
  }
  mfree(arr->strings, sizeof(char*) * arr->count);
  arr->strings = NULL;
  arr->count = 0;
}
