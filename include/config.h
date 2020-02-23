// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#define C_RED "\e[1;31m"
#define C_DARK_RED "\e[0;31m"
#define C_BLUE "\e[1;34m"
#define C_DARK_BLUE "\e[0;34m"
#define C_DARK_GREEN "\e[0;32m"
#define C_YELLOW "\e[0;33m"
#define C_PINK "\e[1;35m"
#define C_NONE "\e[0m"

#define COLOR_WARNING C_YELLOW
#define COLOR_ERROR C_RED
#define COLOR_MESSAGE C_DARK_GREEN
#define COLOR_NUMBER C_BLUE
#define COLOR_NONE C_NONE

#define INPUT_MAX 128

#define HASH_TABLE_INIT_SIZE 3

typedef double obj_number;

#define DECL_NUMBER "number"
#define DECL_VOID "void"

#endif