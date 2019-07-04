// Gherman Maria Irina @ 314CB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct list {
    void *info;
    struct list *next;
} TCell, *TList;

typedef struct stack {
    size_t dim;
    TList top;
} TStack;

typedef struct process {
    // cat din cuanta mai trb sa ruleze ca sa ruleze o cuanta
    // intreaga
    long cuanta_ramasa;
    long memsize;
    long memstart;
    long time;
    long time_remaining;
    int priority;
    int pid;
    unsigned char status;
    TStack *memstack;
} TProc, *TProcess;

typedef struct queue {
    size_t dim;
    TList start;
    TList finish;
} TQueue;

typedef struct memheap {
    void *info;
    struct memheap *next;
    struct memheap *prev;
    int empty_list;
} TMemCell, *TMemList;