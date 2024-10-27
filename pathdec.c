#include <stdio.h>
#include <stdlib.h>

#include "pathdec.h"

extern void set_paths(char **work_path, char **id_path, char **tasks_path) {
    if (work_path != NULL) {
        *work_path = malloc(256);
        snprintf(*work_path, 256, "%s/%s", getenv("HOME"), WORK_PATH);
    }
    if (id_path != NULL) {
        *id_path = malloc(512);
        snprintf(*id_path, 512, "%s/%s/%s", getenv("HOME"), WORK_PATH, 
                 ID_FILENAME);
    }
    if (tasks_path != NULL) {
        *tasks_path = malloc(512);
        snprintf(*tasks_path, 512, "%s/%s/%s", getenv("HOME"), WORK_PATH, 
                 TASKS_FILENAME);
    }
}

extern void unset_paths(char **work_path, char **id_path, char **tasks_path) {
    if (work_path != NULL) {
        free(*work_path);
    }
    if (id_path != NULL) {
        free(*id_path);
    }
    if (tasks_path != NULL) {
        free(*tasks_path);
    }
}
