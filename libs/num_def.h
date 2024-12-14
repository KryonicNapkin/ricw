#ifndef NUM_DEF_H_
#define NUM_DEF_H_ 

#define MAX_PATH_LEN 256
#define MULTI_PATH_LEN(x) ((x < 0) ? 0 : (MAX_PATH_LEN * (x)) + ((x)-1))       /* 1 is for the path separator '/' */
#define SMALL_BUFF_SIZE 32
#define MEDIUM_BUFF_SIZE 64
#define LONG_BUFF_SIZE 128
#define LARGE_BUFF_SIZE 256
#define CNTRL_STR_LEN 2
#define LINE_BUFF_SIZE 1024
#define MAX_CRON_VAL_LEN 16

#endif /* NUM_DEF_H_ */
