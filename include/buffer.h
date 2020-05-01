// buffer.h - string buffers

#ifndef _BUFFER_H
#define _BUFFER_H

struct Buffer {
  char* string;
  int length;
};

void buffer_init(struct Buffer* buff);

int buffer_append(struct Buffer* buff, const char* str);

void buffer_free(struct Buffer* buff);

#endif