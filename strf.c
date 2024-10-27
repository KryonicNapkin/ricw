#include <string.h>
#include <stdlib.h>

#include "strf.h"

/* Function to enclose str in char enclose_char */
char* strenc(char* str, char enclose_char) {
    if (str == NULL) {
        return str;
    }
    size_t len = strlen(str);
    char* dup_str = malloc(len + 2 + 1);

    dup_str[0] = enclose_char;
    for (int i = 1; i <= len; ++i) {
        memset(&dup_str[i], str[i-1], 1);
    }
    dup_str[len+1] = enclose_char;
    dup_str[len+2] = '\0';
    str = dup_str;
    return dup_str;
}

/* Custom strdup */
char* c_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* n_str = malloc(len);
    if (str) {
        memcpy(n_str, str, len);
    }
    return n_str;
}

/* Borrowed function from stack overflow to exclude chars between open_block and close_block */
char* strmbtok(char* s, char* delim, char* open_block, char* close_block) {
    static char *token = NULL;
    char *lead = NULL;
    char *block = NULL;
    int i_block = 0;
    int i_blockIndex = 0;

    if (s != NULL) {
        token = s;
        lead = s;
    } else {
        lead = token;
        if (*token == '\0') {
            lead = NULL;
        }
    }

    while (*token != '\0') {
        if (i_block) {
            if (close_block[i_blockIndex] == *token) {
                i_block = 0;
            }
            token++;
            continue;
        }
        if ((block = strchr(open_block, *token)) != NULL) {
            i_block = 1;
            i_blockIndex = block - open_block;
            token++;
            continue;
        }
        if (strchr(delim, *token) != NULL) {
            *token = '\0';
            token++;
            break;
        }
        token++;
    }
    return lead;
}

/* Truncate str by appending const char* trunc_str at the final_len-strlen(trunc_str) of str */
char* strtrunc(char* str, size_t final_len, const char* trunc_str) {
    if (str == NULL) {
        return str;
    }
    size_t lenght = strlen(str);
    size_t tr_chars = strlen(trunc_str);
    size_t diff_len = lenght-final_len;
    if (tr_chars > lenght) {
        return NULL;
    }
    memmove(str+(final_len-tr_chars), trunc_str, tr_chars);
    str[lenght-diff_len] = '\0';
    return str;
}
