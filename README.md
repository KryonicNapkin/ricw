# ricw
Really Inefficient Cron Wrapper made in C (C11)

### Usage
Long options doesn't work and I don't even want to implement them :(\
If someone create an issue about it I would implement them. But this project
is so useless and unheared of that I don't have to worry about that :)

    $ ricw -h

    Usage: ricw [OPTION]... <args>
    Easy and inefficient way to set up a cron task!

        -a <-o|<name>> <delay> <urg> <desc> <crontime>    add task
        -o                                    if passed, <name> will not be assaign to the task, instead it will be the default name + char
        -c                                    edit the tasks file (!WARNING! DO NOT USE UNLESS YOU KNOW THE CRON SYNTAX)
        -i <name>                             import earlier exported tasks to ricw
        -e <name>                             export current user's configuration of created tasks
        -d <valid_id>                         delete a task with <valid_id> from .tasks file and .crontab file
        -l                                    list all tasks from .tasks file
        -s <init_id>                          set initial id to <init_id>
        -r                                    reset the id counter back to 0
        -x <cntr_str>                         eXpand one the task value (possible values: t/d/n) see FORMAT
        -m <valid_id>                         mark task with an id of <valid_id> as !is_done
        -p                                    print defaults
        -h                                    print this help message

    cntr_str (CONTROL STRING):
    FORMAT: <id><t/d/n>

        <id>               id of the task that you want to select
        <t>                't' if you want to expand the task's crontime
        <d>                'd' if you want to expand the task's description
        <n>                'n' if you want to expand the task's name
