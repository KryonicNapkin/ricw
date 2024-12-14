#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>

#include "libs/path_def.h"
#include "libs/strf.h"
#include "libs/num_def.h"

#define DEFAULT_NAME "Task"
#define CMD_VALS 7
#define TASK_VALS 5
#define CRONTIME_VALS 5
#define MISC_CMD_FLAGS 4
#define LIST_WIDTH 108
#define DELIM " \t\r\a\n"

#define SINGLE_BAR "\u2502"
#define SINGLE_DASH "\u2500"
#define DOUBLE_BAR "\u2551"
#define DOUBLE_DASH "\u2550"
#define DOUBLE_UPLEFT_CORNER "\u2554"
#define DOUBLE_UPRIGHT_CORNER "\u2557"
#define DOUBLE_BAR_SINGLE_DASH_DOWNLEFT_CORNER "\u2559"
#define DOUBLE_BAR_SINGLE_DASH_DOWNRIGHT_CORNER "\u255C"
#define DOUBLE_BAR_SINGLE_DASH_CENTER_RIGHT "\u255F"
#define DOUBLE_BAR_SINGLE_DASH_CENTER_LEFT "\u2562"
#define SINGLE_CROSS "\u253C"
#define SINGLE_BAR_DASH_LAY "\u2534"
#define DOUBLE_BAR_DASH_DOWN "\u2564"

char work_path[MAX_PATH_LEN];
char id_path[MULTI_PATH_LEN(2)];
char tasks_path[MULTI_PATH_LEN(2)];

typedef enum {
    MIN = 0,
    HOUR,
    DMON,
    MON,
    WDAY,
    PREP_CMD,
    SEND_CMD,
    TIME_FLAG,
    DELAY,
    URG_FLAG,
    URGENCY,
    NAME,
    DESC,
    ID = 14,
} idx_val_t;

typedef enum {
    INIT = -1,
    NM,
    CRONTIME,
    DSC,
} expand_value_t;

typedef struct {
    long id;
    expand_value_t val;
} cntrl_t;

typedef enum {
    ERROR = -1,
    LOW,
    NORMAL,
    CRITICAL,
} urg_t;

struct crtime_t {
    char* min;
    char* hour;
    char* dmon;
    char* mon;
    char* wday;
};

typedef struct {
    bool is_done;
    long id;
    struct crtime_t crtime;
    char* prep_cmd;
    struct cmd_t {
        unsigned int delay;
        urg_t urgency;
        char* name;
        char* desc;
    } cmd; 
} task_t;

/* Function prototypes */
/* Check if cronie or fcron is installed is installed*/
int has_crontab(void);
/* Initialize work directory and all files*/
void init_dir(char* work_path, char* id_path, char* tasks_path);
/* Update cron after each task creation */
int update_cron(char* crontab);
/* Get user id before the prep command is written to the .tasks file*/
char* get_prep_cmd(void);
/* Usage */
void usage(void);
/* Init task */
void init_task(task_t* task);
/* is task null */
int is_task_null(task_t task);
/* add task to .tasks file*/
int add_task(const char* filename, task_t task);
/* Add task to .staging file for syntax checking*/
int add_staging(task_t task);
/* List all created tasks */
void list_tasks(const char* filename);
/* Get task */
task_t get_task(const char* filename, long id);
/* Get tasks */
task_t* get_tasks(const char* filename);
/* Count the number of tasks created */
ssize_t task_count(const char* filename);
/* Delete task */
int del_task(const char* filename, long id);
/* Generate a taskname with the init_str + integer */
char* gen_name(const char* init_str);
/* Convert char* time to crontime */
int timeconvr(const char* cron_time, struct crtime_t* crtime);
/* Id system */
long get_id(const char* filename);
int inc_id(const char* filename, long by_what);
int set_id(const char* filename, long init);
#define reset_id(filename) ((void)set_id((filename), (0)))
/* Check if the id is valid */
int valid_id(long id);
/* get urgency name from the number */
const char* get_urgency(urg_t urg_num);
/* load urgency string form to urg_t  */
urg_t load_urgency(const char* str_urg);
/* Variadic system function */
int vsystem(const char* cmd, ...);
/* Copy line by line from str1 to str2 */
int data_copy(const char* src, const char* dest);
/* Expand field of the task for the full display of value */
char* expand_value(char** return_val, const long id, expand_value_t value);
/* Get control string from str */
cntrl_t get_cntrl_str(const char* str);

int main(int argc, char* argv[]) {
    task_t task;
    init_task(&task);
    struct crtime_t crontime;
    char* endptr = NULL;
    int is_crontab = has_crontab();
    int status = 0;
    long id;
    unsigned int delay;
    urg_t urgency;

    if (is_crontab == -1) {
        fprintf(stderr, "Error: Failed to run command!\n");
        exit(EXIT_FAILURE);
    } else if (!is_crontab) {
        fprintf(stdout, "Crontab command not installed. \nPlease check if you have some kind of cron implementation installed i.e. cronie or fcron\n");
        exit(EXIT_SUCCESS);
    }

    set_paths(&work_path, &id_path, &tasks_path);

    DIR* work_dir = opendir(work_path);
    if (work_dir == NULL && errno == ENOENT) {
        init_dir(work_path, id_path, tasks_path);
        set_id(id_path, 0);
    } else if (work_dir && (access(id_path, F_OK) == -1 || access(tasks_path, F_OK) == -1)) {
        FILE* id_f = fopen(id_path, "a");
        FILE* tasks_f = fopen(tasks_path, "w");
        if (id_f == NULL) {
            fprintf(stderr, "Error: Cannot create %s\n%s\n", id_path, strerror(errno));
            exit(EXIT_FAILURE);
        } 
        if (tasks_f == NULL) {
            fprintf(stderr, "Error: Cannot create %s\n%s\n", tasks_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        fclose(tasks_f);
        fclose(id_f);
        set_id(id_path, 0);
    } else if (work_dir) {
    } else {
        fprintf(stderr, "Error: Cannot open %s\n%s\n", work_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (argc >= 2 && argv[1][0] == '-') {
        switch (argv[1][1]) {
            case 'a':;
                bool has_name = true;

                if (argc != 7) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                if (!strncmp(argv[2], "-o", 2)) {
                    has_name = false;
                }
                delay = (unsigned int)strtol(argv[3], &endptr, 10);
                if (argv[3] == endptr && delay == 0) {
                    fprintf(stderr, "Error: Cannot convert delay to a number!\n");
                    exit(EXIT_FAILURE);
                }
                urgency = (urg_t)strtol(argv[4], &endptr, 10);
                if (argv[4] == endptr && urgency == 0) {
                    fprintf(stderr, "Error: Cannot convert urgency to a number!\n");
                    exit(EXIT_FAILURE);
                }
                if (!timeconvr(argv[6], &crontime)) {
                    fprintf(stderr, "Error: Cannot convert time to cron readable format!\n");
                    exit(EXIT_FAILURE);
                }
                task = (task_t) {
                    .id = get_id(id_path),
                    .crtime = crontime,
                    .prep_cmd = get_prep_cmd(),
                    .cmd.desc = strenc(argv[5], '"'),
                    .cmd.delay = delay,
                    .cmd.name = (has_name ? strenc(argv[2], '"') : strenc(gen_name(DEFAULT_NAME), '"')),
                    .cmd.urgency = urgency,
                    .is_done = false,
                };
                status = add_staging(task);
                if (remove(strcat(work_path, "/" STG_FILENAME)) == -1) {
                    fprintf(stderr, "Error: Cannot remove %s\n%s\n", work_path, strerror(errno));
                    exit(EXIT_FAILURE);
                };
                if (status) {
                    fprintf(stderr, "Error: Bad Syntax!\n");
                    exit(EXIT_FAILURE);
                }
                add_task(tasks_path, task);
                inc_id(id_path, 1);
                goto update_cron;
                break;
            case 'd':
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                id = strtol(argv[2], &endptr, 10); 
                if (endptr == argv[2] && id == 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (!valid_id(id)) {
                    fprintf(stderr, "Error: Invalid id of task passed!\n");
                    exit(EXIT_FAILURE);
                }
                del_task(tasks_path, id);
                goto update_cron;
                break;
            case 'l':
                if (task_count(tasks_path) == 0) {
                    fprintf(stdout, "You have not created any tasks yet!\n");
                    exit(EXIT_SUCCESS);
                }
                list_tasks(tasks_path);
                break;
            case 'r':
                reset_id(id_path);
                break;
            case 's':
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                id = strtol(argv[2], &endptr, 10);
                if (endptr == argv[2] && id == 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                set_id(id_path, id);
                break;
            case 'm':
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                id = strtol(argv[2], &endptr, 10);
                if (endptr == argv[2] && id == 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (!valid_id(id)) {
                    fprintf(stderr, "Error: Invalid id passed as an argument!\n");
                    exit(EXIT_FAILURE);
                }
                task = get_task(tasks_path, id);
                if (is_task_null(task)) {
                    fprintf(stderr, "Error: Could not obtain informations about \
                                    the task with id: %ld\n", id);
                    exit(EXIT_FAILURE);
                }
                del_task(tasks_path, id);
                task.is_done = !task.is_done;
                add_task(tasks_path, task);
                goto update_cron;
                break;
            case 'i':
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                if (access(argv[2], F_OK) == -1) {
                    fprintf(stderr, "Error: Cannot find %s :%s\n", argv[2], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                char new_tasks[MULTI_PATH_LEN(2)+MEDIUM_BUFF_SIZE];
                sprintf(new_tasks, "%s%s", tasks_path, "_back");
                if (data_copy(tasks_path, new_tasks) == -1) {
                    fprintf(stderr, "Error: Cannot create a backup of the tasks: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (rename(argv[2], tasks_path) == -1) {
                    fprintf(stderr, "Error: Cannot import tasks: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            case 'e':;
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                if (data_copy(tasks_path, argv[2]) == -1) {
                    fprintf(stderr, "Error: Cannot create a backup: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
                vsystem("vim %s", tasks_path);
                goto update_cron;
                break;
            case 'p':
                fprintf(stdout, "default name    : %s\n", DEFAULT_NAME);
                break;
            case 'x':
                if (argc != 3) {
                    fprintf(stderr, "Error: Invalid number of arguments!\n");
                    exit(EXIT_FAILURE);
                }
                char buff[SMALL_BUFF_SIZE];
                char* str;
                cntrl_t cntrl_str = get_cntrl_str(argv[2]);
                if (cntrl_str.id == -1 && cntrl_str.val == -1) {
                    fprintf(stderr, "Error: Cannot obtain the full value!\n");
                    exit(EXIT_FAILURE);
                }
                expand_value(&str, cntrl_str.id, cntrl_str.val);
                printf("Task with id: %ld\n", cntrl_str.id);
                switch (cntrl_str.val) {
                    case NM:
                        strcpy(buff, "name");
                        break;
                    case DSC:
                        strcpy(buff, "desc");
                        break;
                    case CRONTIME:
                        strcpy(buff, "crontime");
                        break;
                    case INIT:
                    default:
                        fprintf(stderr, "Error: Cannot obtain the full value!\n");
                        free(str);
                        exit(EXIT_FAILURE);
                        break;
                }
                printf("Full %s is %s\n", buff, str);
                free(str);
                break;
            case 'h':
                usage();
                break;
            default:
                fprintf(stderr, "Error: Invalid argument %s\n", argv[1]);
                exit(EXIT_FAILURE);
                break;
        }
    } 
    return status;
    update_cron:
        status = update_cron(tasks_path);
}

int has_crontab(void) {
    char buff[LARGE_BUFF_SIZE];

    FILE* fp = popen("which crontab", "r");
    if (fp == NULL) {
        return -1;
    }

    while (fgets(buff, sizeof(buff), fp) != NULL) {
        if (!strncmp(buff, "crontab not found", 17)) {
            pclose(fp);
            return 0;
        } else {
            pclose(fp);
            return 1;
        }
    }

    pclose(fp);
    return -1;
}

void init_dir(char* work_path, char* id_path, char* tasks_path) {
    mkdir(work_path, 0777);
    FILE* id_f = fopen(id_path, "w");
    FILE* task_f = fopen(tasks_path, "a");
    if (id_f == NULL || task_f == NULL) {
        fprintf(stderr, "Error: Cannot open %s or %s\n%s\n", id_path,
                tasks_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(id_f);
    fclose(task_f);
}

int update_cron(char* crontab) {
    int ret_status;
    char* const argv[] = {"crontab", crontab, NULL};
    pid_t pid = fork();
    if (pid == -1) {
       fprintf(stderr, "Error: Cannot fork process!\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execv("/usr/bin/crontab", argv);
    }
    waitpid(-1, &ret_status, 0);
    return WEXITSTATUS(ret_status);
}

int vsystem(const char* cmd, ...) {
    char buff[LARGE_BUFF_SIZE];
    va_list ap;
    va_start(ap, cmd);
    vsnprintf(buff, sizeof(buff), cmd, ap);
    va_end(ap);

    return system(buff);
}

void usage(void) {
    fprintf(stdout, "Usage: ricw [OPTION]... <args>\n");
    fprintf(stdout, "Easy and inefficient way to set up a cron task!\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "    -a <-o|<name>> <delay> <urg> <desc> <crontime>    add task \n");
    fprintf(stdout, "    -o                                    if passed, <name> will not be assaign to the task, instead it will be the default name + char\n");
    fprintf(stdout, "    -c                                    edit the tasks file (!WARNING! DO NOT USE UNLESS YOU KNOW THE CRON SYNTAX)\n");
    fprintf(stdout, "    -i <name>                             import earlier exported tasks to ricw\n");
    fprintf(stdout, "    -e <name>                             export current user's configuration of created tasks\n");
    fprintf(stdout, "    -d <valid_id>                         delete a task with <valid_id> from .tasks file and .crontab file\n");
    fprintf(stdout, "    -l                                    list all tasks from .tasks file\n");
    fprintf(stdout, "    -s <init_id>                          set initial id to <init_id>\n");
    fprintf(stdout, "    -r                                    reset the id counter back to 0\n");
    fprintf(stdout, "    -x <cntrl_str>                        eXpand one the task value (possible values: t/d/n) see FORMAT\n");
    fprintf(stdout, "    -m <valid_id>                         mark task with an id of <valid_id> as !is_done\n");
    fprintf(stdout, "    -p                                    print defaults\n");
    fprintf(stdout, "    -h                                    print this help message\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "cntrl_str (CONTROL STRING):\n");
    fprintf(stdout, "FORMAT: <id><t/d/n>\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "    <id>               id of the task that you want to select\n");
    fprintf(stdout, "    <t>                't' if you want to expand the task's crontime\n");
    fprintf(stdout, "    <d>                'd' if you want to expand the task's description\n");
    fprintf(stdout, "    <n>                'n' if you want to expand the task's name\n");

}

int add_task(const char* filename, task_t task) {
    const char* command = "notify-send";
    FILE* fp = fopen(filename, "a");
    if (fp == NULL) {
        return errno;
    }
    if (task.is_done) {
        fprintf(fp, "#- ");
    }
    fprintf(fp, "%s %s %s %s %s ", task.crtime.min, task.crtime.hour, 
            task.crtime.dmon, task.crtime.mon, task.crtime.wday);
    fprintf(fp, "%s ", task.prep_cmd);
    fprintf(fp, "%s %s %u %s %s %s %s # %ld\n", command, "-t", task.cmd.delay, 
            "-u", get_urgency(task.cmd.urgency), task.cmd.name, task.cmd.desc, 
            task.id);
    fclose(fp);
    return 1;
}

char* gen_name(const char* init_str) {
    char* name = malloc(SMALL_BUFF_SIZE);
    const char* charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char c = *(charset+(get_id(id_path) % strlen(charset)));
    snprintf(name, SMALL_BUFF_SIZE, "%s%c", init_str, c);
    return name;
}

int del_task(const char* filename, long id) {
    const char* tmp_filename = ".tmptasks";
    char buff[LINE_BUFF_SIZE];
    int task_id = 0;
    FILE* task_f = fopen(filename, "r");
    FILE* temp_f = fopen(tmp_filename, "a");
    if (temp_f == NULL) {
        return -1;
    }
    if (task_f == NULL) {
        return errno;
    }
    while (fgets(buff, sizeof(buff), task_f) != NULL) {
        char* str = c_strdup(buff);
        char* it = strrchr(str, '#');
        sscanf(it, "# %d\n", &task_id);
        if (task_id != id) {
            fputs(buff, temp_f);
        }
        free(str);
        continue;
    }
    fclose(temp_f);
    if (rename(tmp_filename, filename) == -1) {
        return -1;
    }
    fclose(task_f);
    return 1;
}

task_t* get_tasks(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }
    ssize_t num = task_count(filename);
    if (!num || num == -1) {
        return NULL;
    }
    task_t* tasks = malloc(num * sizeof(task_t));
    int num_vals = TASK_VALS + CMD_VALS + MISC_CMD_FLAGS;
    char** vals = malloc(num_vals * sizeof(char*));
    struct crtime_t crontime;

    int idx = 0, tindex = 0;
    char buff[LINE_BUFF_SIZE];
    char* token;
    while (fgets(buff, sizeof(buff), fp) != NULL) {
        for (int i = 0; i < num_vals; ++i) {
            vals[i] = malloc(LONG_BUFF_SIZE);
        }
        int add_index = 0;
        if (!strncmp(buff, "#-", 2)) {
            add_index = 1;
        } 
        token = strmbtok(buff, DELIM, "\"", "\"");
        while (token != NULL) {
            vals[idx] = c_strdup(token);
            token = strmbtok(NULL, DELIM, "\"", "\"");
            idx++;
        }
        idx = 0;
        crontime = (struct crtime_t) {
            .min = c_strdup(vals[add_index+MIN]),
            .hour = c_strdup(vals[add_index+HOUR]),
            .dmon = c_strdup(vals[add_index+DMON]),
            .mon = c_strdup(vals[add_index+MON]),
            .wday = c_strdup(vals[add_index+WDAY]),
        };
        tasks[tindex] = (task_t) {
            .id = strtol(vals[add_index+ID], NULL, 10),
            .crtime = crontime,
            .prep_cmd = c_strdup(vals[add_index+PREP_CMD]),
            .cmd.delay = (unsigned int)strtol(vals[add_index+DELAY], NULL, 10),
            .cmd.urgency = (unsigned int)load_urgency(vals[add_index+URGENCY]),
            .cmd.name = c_strdup(vals[add_index+NAME]),
            .cmd.desc = c_strdup(vals[add_index+DESC]),
            .is_done = add_index,
        };
        for (int x = 0; x < num_vals; ++x) {
            free(vals[x]);
        }
        tindex++;
    }
    free(vals);
    fclose(fp);
    return tasks;
}

void list_tasks(const char* filename) {
    task_t* tasks = get_tasks(filename);
    ssize_t tasks_c = task_count(filename);
    if (!tasks_c || tasks_c == -1) {
        fprintf(stderr, "Error: Cannot list tasks!\n");
        free(tasks);
        exit(EXIT_FAILURE);
    }
    const int sep_arr[] = {5, 11, 37, 47, 58, 70, 92};

    printf(DOUBLE_UPLEFT_CORNER);
    for (int i = 0, d = 0; i < LIST_WIDTH-CMD_VALS; ++i) {
        if (sep_arr[d] == i) {
            printf(DOUBLE_BAR_DASH_DOWN);
            ++d;
        }
        printf(DOUBLE_DASH);
    }

    printf(DOUBLE_UPRIGHT_CORNER "\n");
    printf(DOUBLE_BAR " Nth "SINGLE_BAR"  ID  "SINGLE_BAR" MIN  HOUR DMON MON  WDAY "SINGLE_BAR" URGENCY  "SINGLE_BAR" DELAY(ms) "SINGLE_BAR"    NAME    "SINGLE_BAR"         DESC         "SINGLE_BAR" IS_DONE \u2551\n");

    printf(DOUBLE_BAR_SINGLE_DASH_CENTER_RIGHT);
    for (int i = 0, d = 0; i < LIST_WIDTH-CMD_VALS; ++i) {
        if (sep_arr[d] == i) {
            printf(SINGLE_CROSS);
            ++d;
        }
        printf(SINGLE_DASH);
    }

    printf(DOUBLE_BAR_SINGLE_DASH_CENTER_LEFT);
    printf("\n");
    for (int i = 1; i <= tasks_c; ++i) {
        if (strlen(tasks[i-1].cmd.name) > 10) {
            strtrunc(tasks[i-1].cmd.name, 10, "...\"");
        } 
        if (strlen(tasks[i-1].cmd.desc) > 20) {
            strtrunc(tasks[i-1].cmd.desc, 20, "...\"");
        } 
        if (strlen(tasks[i-1].crtime.min) > 4) {
            strtrunc(tasks[i-1].crtime.min, 4, ">");
        }
        if (strlen(tasks[i-1].crtime.mon) > 4) {
            strtrunc(tasks[i-1].crtime.mon, 4, ">");
        }
        if (strlen(tasks[i-1].crtime.hour) > 4) {
            strtrunc(tasks[i-1].crtime.hour, 4, ">");
        }
        if (strlen(tasks[i-1].crtime.dmon) > 4) {
            strtrunc(tasks[i-1].crtime.dmon, 4, ">");
        }
        if (strlen(tasks[i-1].crtime.wday) > 4) {
            strtrunc(tasks[i-1].crtime.wday, 4, ">");
        }
        printf(DOUBLE_BAR " %03d "SINGLE_BAR" %04ld "SINGLE_BAR" %-4s %-4s %-4s %-4s %-4s "SINGLE_BAR" %-8s "SINGLE_BAR" %-9d "SINGLE_BAR" %-10s "SINGLE_BAR" %-20s "SINGLE_BAR"    %d    \u2551\n", 
               i, tasks[i-1].id, tasks[i-1].crtime.min, tasks[i-1].crtime.hour, tasks[i-1].crtime.dmon,
               tasks[i-1].crtime.mon,tasks[i-1].crtime.wday, get_urgency(tasks[i-1].cmd.urgency), 
               (tasks[i-1].cmd.delay), tasks[i-1].cmd.name, tasks[i-1].cmd.desc,
               tasks[i-1].is_done);
    }

    printf(DOUBLE_BAR_SINGLE_DASH_DOWNLEFT_CORNER);
    for (int i = 0, d = 0; i < LIST_WIDTH-CMD_VALS; ++i) {
        if (sep_arr[d] == i) {
            printf(SINGLE_BAR_DASH_LAY);
            ++d;
        }
        printf(SINGLE_DASH);
    }
    printf(DOUBLE_BAR_SINGLE_DASH_DOWNRIGHT_CORNER "\n");
    free(tasks);
}

ssize_t task_count(const char* filename) {
    ssize_t num_tasks = 0;
    char buff[LINE_BUFF_SIZE];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }
    while (fgets(buff, sizeof(buff), fp) != NULL) {
        num_tasks++;
    }
    fclose(fp);
    return num_tasks;
}

long get_id(const char* filename) {
    char buff[SMALL_BUFF_SIZE];
    char* endptr = NULL;
    long id = 0;
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return errno;
    }
    if (fgets(buff, sizeof(buff), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    buff[strcspn(buff, "\n")] = '\0';
    id = strtol(buff, &endptr, 10);
    if (buff == endptr) {
        fclose(fp);
        return errno;
    }
    fclose(fp);
    return id;
}

int inc_id(const char* filename, long by_what) {
    long id = get_id(filename);
    FILE* fp = fopen(filename, "w+");
    if (fp == NULL) {
        return errno;
    }
    rewind(fp);
    fprintf(fp, "%ld\n", id + by_what);
    fflush(fp);
    fclose(fp);
    return 1;
}

int set_id(const char* filename, long init) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        return errno;
    }
    rewind(fp);
    fprintf(fp, "%ld\n", init);
    fflush(fp);
    fclose(fp);
    return 1;
}

int valid_id(long id) {
    task_t* tasks = get_tasks(tasks_path);
    ssize_t tasks_c = task_count(tasks_path);
    if (!tasks_c || tasks_c == -1) {
        free(tasks);
        return -1;
    }
    
    for (int i = 0; i < tasks_c; ++i) {
        if (tasks[i].id == id) {
            free(tasks);
            return 1;
        }
    }
    free(tasks);
    return 0;
}

int timeconvr(const char* str, struct crtime_t* crontime) {
    /* Cron time format "*;*;*;*;*" */
    char** crtime = malloc(CRONTIME_VALS * sizeof(char*));
    for (int i = 0; i < CRONTIME_VALS; ++i) {
        crtime[i] = malloc(MAX_CRON_VAL_LEN);
    }
    char* str_time = c_strdup(str);
    char* token;
    int count = 0;
    int idx = 0;
    size_t len = strlen(str_time);

    if (len < 9) {
        goto free_mem;
        return 0;
    }
    for (int i = 0; i < len; ++i) {
        if (str_time[i] == ';') {
            ++count;
        }
    }
    if (count != 4) {
        goto free_mem;
        return 0;
    }
    token = strtok(str_time, DELIM ";");
    while (token != NULL) {
        crtime[idx] = c_strdup(token);
        token = strtok(NULL, DELIM ";");
        idx++;
    }
    crontime->min = crtime[0];
    crontime->hour = crtime[1];
    crontime->dmon = crtime[2];
    crontime->mon = crtime[3];
    crontime->wday = crtime[4];

    return 1;
    free_mem:
        free(str_time);
        for (int x = 0; x < CRONTIME_VALS; ++x) {
            free(crtime[x]);
        }
        free(crtime);
}

urg_t load_urgency(const char* str_urg) {
    if (!strncmp(str_urg, "normal", 6)) {
        return NORMAL;
    } else if (!strncmp(str_urg, "low", 3)) {
        return LOW;
    } else if (!strncmp(str_urg, "critical", 8)) {
        return CRITICAL;
    }
    return ERROR;
}

const char* get_urgency(urg_t urg_num) {
    switch (urg_num) {
        case LOW:
            return "low";
            break;
        case NORMAL:
            return "normal";
            break;
        case CRITICAL:
            return "critical";
            break;
        default:
            return NULL;
        break;
    }
}

task_t get_task(const char* filename, long id) {
    task_t task;
    init_task(&task);
    struct crtime_t crontime;
    char* token;
    int task_id = -1;
    int num_vals = TASK_VALS + CMD_VALS + MISC_CMD_FLAGS;
    char** vals = malloc(num_vals * sizeof(char*));
    char buff[LINE_BUFF_SIZE];
    int idx = 0;
    FILE* tasks_f = fopen(filename, "r");
    if (tasks_f == NULL) {
        return task;
    }

    while (fgets(buff, sizeof(buff), tasks_f) != NULL) {
        for (int i = 0; i < num_vals; ++i) {
            vals[i] = malloc(LONG_BUFF_SIZE);
        }
        int add_index = 0;
        if (!strncmp(buff, "#-", 2)) {
            add_index = 1;
        }
        char* str = c_strdup(buff);
        token = strmbtok(buff, DELIM, "\"", "\"");
        while (token != NULL) {
            vals[idx] = c_strdup(token);
            token = strmbtok(NULL, DELIM, "\"", "\"");
            idx++;
        }
        idx = 0;
        char* it = strrchr(str, '#');
        sscanf(it, "# %d\n", &task_id);
        if (id == task_id) {
            crontime = (struct crtime_t) {
                .min = c_strdup(vals[add_index+MIN]),
                .hour = c_strdup(vals[add_index+HOUR]),
                .dmon = c_strdup(vals[add_index+DMON]),
                .mon = c_strdup(vals[add_index+MON]),
                .wday = c_strdup(vals[add_index+WDAY]),
            };
            task = (task_t) {
                .id = strtol(vals[add_index+ID], NULL, 10),
                .crtime = crontime,
                .prep_cmd = c_strdup(vals[add_index+PREP_CMD]),
                .cmd.delay = strtol(vals[add_index+DELAY], NULL, 10),
                .cmd.urgency = load_urgency(vals[add_index+URGENCY]),
                .cmd.name = c_strdup(vals[add_index+NAME]),
                .cmd.desc = c_strdup(vals[add_index+DESC]),
                .is_done = add_index,
            };
        }
        for (int x = 0; x < num_vals; ++x) {
            free(vals[x]);
        }
        free(str);
    }
    free(vals);
    fclose(tasks_f);
    return task;
}

char* get_prep_cmd(void) {
    char* prep_cmd = malloc(LONG_BUFF_SIZE);
    unsigned int user_id = 0;

    FILE* pp = popen("id -u", "r");
    if (pp == NULL) {
        free(prep_cmd);
        return NULL;
    }
    if (fscanf(pp, "%u", &user_id) == -1) {
        free(prep_cmd);
        pclose(pp);
        return NULL;
    }
    snprintf(prep_cmd, LONG_BUFF_SIZE, "XDG_RUNTIME_DIR=/run/user/%u", user_id);
    pclose(pp);
    return prep_cmd;
}

int add_staging(task_t task) {
    char staging[MAX_PATH_LEN+MEDIUM_BUFF_SIZE];
    snprintf(staging, sizeof(staging), "%s/%s", work_path, STG_FILENAME);
    add_task(staging, task);
    return update_cron(staging);
}

int data_copy(const char* src, const char* dest) {
    FILE* source_f = fopen(src, "r");
    if (source_f == NULL) {
        return -1;
    }
    FILE* destination_f = fopen(dest, "w");
    if (destination_f == NULL) {
        return -1;
    }
    char buff[LINE_BUFF_SIZE];
    while (fgets(buff, sizeof(buff), source_f) != NULL) {
        fputs(buff, destination_f);
    }
    fclose(source_f);
    fclose(destination_f);
    return 1;
}

/* Init task */
void init_task(task_t* task) {
    task->id = -1;
    task->is_done = false;
    task->prep_cmd = NULL;
}

/* is task null */
int is_task_null(task_t task) {
    if (task.id == -1 && task.prep_cmd == NULL && task.is_done == false) {
        return 1;
    }
    return 0;
}

char* expand_value(char** return_val, const long id, expand_value_t value) {
    char buff[SMALL_BUFF_SIZE];
    if (!valid_id(id)) {
        return *return_val;
    }
    task_t task = get_task(tasks_path, id);
    if (is_task_null(task)) {
        return *return_val;
    }
    switch (value) {
        case NM:
            *return_val = c_strdup(task.cmd.name);
            break;
        case CRONTIME:
            snprintf(buff, sizeof(buff), "%s;%s;%s;%s;%s", task.crtime.min,
                     task.crtime.hour, task.crtime.dmon, task.crtime.mon, 
                     task.crtime.wday);
            *return_val = c_strdup(buff);
            break;
        case DSC:
            *return_val = c_strdup(task.cmd.desc);
            break;
        case INIT:
        default:
            return *return_val;
            break;
    }
    return *return_val;
}

cntrl_t get_cntrl_str(const char* str) {
    cntrl_t control_str = {
        .id = -1,
        .val = INIT,
    };
    size_t len = strlen(str);
    char* endptr = NULL;
    long id;
    if (len > CNTRL_STR_LEN || len == 0) {
        return control_str;
    } 
    static const char* valid_chars = "tdn";
    id = strtol(str, &endptr, 10);
    if (id == 0 && str == endptr) {
        return control_str;
    } 
    if (!valid_id(id)) {
        return control_str;
    }
    for (int i = 0; i < strlen(valid_chars); ++i) {
        if (*endptr == valid_chars[i]) {
            switch (valid_chars[i]) {
                case 't':
                    control_str.val = CRONTIME;
                    break;
                case 'd':
                    control_str.val = DSC;
                    break;
                case 'n':
                    control_str.val = NM;
                    break;
                default:
                    return control_str;
                    break;
            }
            control_str.id = id;
            return control_str; 
        }
    }
    return control_str;
}
