// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#define SI_VERSION "0.1"
#define PROMPT "> "
#define MESSAGE_TITLE "si - simple interpreter (version " SI_VERSION ")"

#if defined(USE_COLORS)

#define C_RED "\e[1;31m"
#define C_DARK_RED "\e[0;31m"
#define C_BLUE "\e[1;34m"
#define C_DARK_BLUE "\e[0;34m"
#define C_GREEN "\e[1;32m"
#define C_DARK_GREEN "\e[0;32m"
#define C_CYAN "\e[1;36m"
#define C_DARK_CYAN "\e[0;36m"
#define C_YELLOW "\e[1;33m"
#define C_DARK_YELLOW "\e[0;33m"
#define C_PINK "\e[1;35m"
#define C_GRAY "\e[0;90m"
#define C_NONE "\e[0m"

#define COLOR_WARNING C_PINK
#define COLOR_ERROR C_RED
#define COLOR_MESSAGE C_DARK_GREEN
#define COLOR_NUMBER C_DARK_YELLOW
#define COLOR_STRING C_DARK_GREEN
#define COLOR_TYPE C_DARK_CYAN
#define COLOR_NIL C_GRAY
#define COLOR_NONE C_NONE

#else

#define COLOR_WARNING
#define COLOR_ERROR
#define COLOR_MESSAGE
#define COLOR_NUMBER
#define COLOR_STRING
#define COLOR_TYPE
#define COLOR_NIL
#define COLOR_NONE

#endif

#define INPUT_MAX 128

#define PATH_LENGTH_MAX 512

#define HASH_TABLE_INIT_SIZE 7

#endif
