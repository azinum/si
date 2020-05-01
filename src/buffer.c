// buffer.c

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "mem.h"
#include "buffer.h"

void buffer_init(struct Buffer* buff) {
  assert(buff != NULL);
  buff->string = NULL;
  buff->length = 0;
}

int buffer_append(struct Buffer* buff, const char* str) {
  assert(buff != NULL);
  int str_len = strlen(str);
  if (!buff->string) {
    buff->length = str_len;
    buff->string = mmalloc(sizeof(char) * str_len);
    strncpy(buff->string, str, str_len);
    return 0;
  }
  int new_length = buff->length + str_len;
  char* temp = mrealloc(buff->string, buff->length * sizeof(char), sizeof(char) * new_length);
  if (!temp)
    return -1;
  buff->string = temp;
  strncpy(&buff->string[buff->length], str, str_len);
  buff->length = new_length;
  return 0;
}

void buffer_free(struct Buffer* buff) {
  assert(buff != NULL);
  mfree(buff->string, buff->length);
  buff->string = NULL;
  buff->length = 0;
}