#include <stdio.h>      /* For snprintf */
#include <stdlib.h>     /* For getenv */

#include "../libs/num_def.h"    /* Number definitions for the lenght of paths*/

#include "../libs/path_def.h"    /* Global path variables */

/* this functions sets the default path for all the files 
 * and directories for this program to work
 *
 * work_path: is the default directory for all the files that this program uses
 * id_path: path to a file that holds the index of next to be created task
 * tasks_path: path to a file in which all user created tasks are kept
 */
extern void set_paths(char (*work_path)[], char (*id_path)[], char (*tasks_path)[]) {
    if (work_path != NULL) {
        snprintf(*work_path, MAX_PATH_LEN, "%s/%s", getenv("HOME"), WORK_PATH);
    }
    if (id_path != NULL) {
        snprintf(*id_path, MULTI_PATH_LEN(2), "%s/%s/%s", getenv("HOME"), WORK_PATH, 
                 ID_FILENAME);
    }
    if (tasks_path != NULL) {
        snprintf(*tasks_path, MULTI_PATH_LEN(2), "%s/%s/%s", getenv("HOME"), WORK_PATH, 
                 TASKS_FILENAME);
    }
}
