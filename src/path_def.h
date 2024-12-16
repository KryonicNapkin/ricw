#ifndef PATH_DEF_H_
#define PATH_DEF_H_

#define WORK_PATH ".local/share/ricw"
#define TASKS_FILENAME "tasks"
#define ID_FILENAME "id"
#define STG_FILENAME ".staging"

#include "num_def.h"

extern char work_path[MAX_PATH_LEN];
extern char id_path[MULTI_PATH_LEN(2)];
extern char tasks_path[MULTI_PATH_LEN(2)];

extern void set_paths(char (*work_path)[], char (*id_path)[], char (*tasks_path)[]);

#endif /* PATH_DEF_H_ */
