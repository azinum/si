// strarr.h - string array

#ifndef _STRARR_H
#define _STRARR_H

struct Str_arr {
  char** strings;
  int count;
};

void strarr_init(struct Str_arr* arr);

int strarr_append(struct Str_arr* arr, const char* str);

char* strarr_top(struct Str_arr* arr);

void strarr_free(struct Str_arr* arr);

#endif