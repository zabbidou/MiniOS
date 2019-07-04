// Gherman Maria Irina @ 314CB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "struct.h"
#include "functions.c"

int main(int argc, char *argv[]) {
    char buffer[1000];
    int i = 1;
    int pid = 0;
    char *param;
    long T = 0, memsize, time, info;
    int priority;

    TProcess cur_proc = NULL, aux = NULL;
    cur_proc = malloc(sizeof(TProc));
    if (!cur_proc) {
        return -2;
    }

    cur_proc = NULL;
    char pid_list[32768] = {0};

    TMemList L = calloc(1, sizeof(TMemCell));
    if (!L) {
        return -2;
    }

    L->empty_list = 1;

    if (argc != 3) {
        return -1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        return -1;
    }

    FILE *output = fopen(argv[2], "w");
    if (!output) {
        fclose(input);
        return -1;
    }

    TQueue *wait = InitQ(sizeof(TProc));
    TQueue *finish = InitQ(sizeof(TProc));

    fscanf(input, "%li", &T);

    while (fgets(buffer, 1000, input)) {
        i++;
        param = strtok(buffer, " ");

        if (strcmp(param, "add") == 0) {
            param = strtok(NULL, " ");
            memsize = atol(param);
            param = strtok(NULL, " ");
            time = atol(param);
            param = strtok(NULL, "\n");
            priority = atoi(param);

            aux = build_process(&L, memsize, time, priority, pid_list, 
                                output, T);

            if (aux == NULL) {
                continue;
            }

            if (cur_proc == NULL) {
                memcpy(&cur_proc, &aux, sizeof(aux));
            } else {
                add_waiting(wait, aux);
            }

            fprintf(output, "Process created successfully: PID: %d, ", 
                            aux->pid);
            fprintf(output, "Memory starts at 0x%lx.\n", aux->memstart);
            continue;
        }

        if (strcmp(param, "get") == 0) {
            param = strtok(NULL, "\n");
            pid = atoi(param);
            get(pid, cur_proc, wait, finish, output);
            continue;
        }

        if (strcmp(param, "run") == 0) {
            param = strtok(NULL, "\n");
            time = atoi(param);
            cur_proc = run(&L, cur_proc, wait, finish, time, T, pid_list);
            continue;
        }

        if (strcmp(param, "print") == 0) {
            param = strtok(NULL, " ");

            if (strncmp(param, "waiting", 7) == 0) {
                fprintf(output, "Waiting queue:\n");
                PrintQueue(wait, output, "remaining_time");
                continue;
            }
            
            if (strncmp(param, "finished", 8) == 0) {
                fprintf(output, "Finished queue:\n");
                PrintQueue(finish, output, "executed_time");
                continue;
            }

            if (strcmp(param, "stack") == 0) {
                param = strtok(NULL, "\n");
                pid = atoi(param);
                aux = find_process(cur_proc, wait, finish, pid);

                if (aux == NULL) {
                    fprintf(output, "PID %d not found.\n", pid);
                }
                
                int err = print_stack(aux, output);

                if (err == -1) {
                    fprintf(output, "Empty stack PID %d.\n", pid);
                }

                continue;
            }
        }

        if (strcmp(param, "push") == 0) {
            param = strtok(NULL, " ");
            pid = atoi(param);

            param = strtok(NULL, "\n");
            info = atol(param);
            aux = find_process(cur_proc, wait, wait, pid);

            if (aux == NULL) {
                fprintf(output, "PID %d not found.\n", pid);                
            }

            push_for_process(aux, &info, output);
            continue;
        }

        if (strcmp(param, "pop") == 0) {
            param = strtok(NULL, "\n");
            pid = atoi(param);
            aux = find_process(cur_proc, wait, wait, pid);

            if (aux == NULL) {
                fprintf(output, "PID %d not found.\n", pid);
                continue;
            }

            pop_for_process(aux, output);
            continue;
        }

        if (strncmp(param, "finish", 6) == 0) {
            time = finish_running(L, cur_proc, wait);
            fprintf(output, "Total time: %ld\n", time);
            continue;
        }
    }
    free(cur_proc);
    free(L);
    fclose(output);
    fclose(input);
}