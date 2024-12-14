/* Silly library for ricw additional string functions */
#ifndef STRF_H_
#define STRF_H_

#include <stdio.h>

/* Function to enclose str in char enclose_char */
char* strenc(char* str, char enclose_char);
/* Custom strdup */
char* c_strdup(const char* str);
/* Borrowed function from stack overflow to exclude chars between open_block and close_block */
char* strmbtok(char* s, char* delim, char* open_block, char* close_block);
/* Truncate str by appending const char* trunc_str at the final_len-strlen(trunc_str) of str */
char* strtrunc(char* str, size_t final_len, const char* trunc_str);

#endif /* STRF_H_ */
