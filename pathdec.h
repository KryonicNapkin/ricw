#ifndef PATHDEC_H_
#define PATHDEC_H_

#define WORK_PATH ".local/share/ricw"
#define TASKS_FILENAME "tasks"
#define ID_FILENAME "id"
#define STG_FILENAME ".staging"

extern char* work_path;
extern char* id_path;
extern char* tasks_path;

extern void set_paths(char** work_path, char** id_path, char** tasks_path);
extern void unset_paths(char** work_path, char** id_path, char** tasks_path); 

#endif /* PATHDEC_H_ */
