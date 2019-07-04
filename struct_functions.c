//#include "struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void* InitS(size_t d) {
    TStack *s = calloc(1, sizeof(TStack));

    if (!s) {
        printf("plm?\n");
        return NULL;
    }

    s->dim = d;
    s->top = NULL;
    return (void*)s;
}

void* InitQ(size_t d) {
    TQueue *q = calloc(1, sizeof(TQueue));

    if (!q) {
        return NULL;
    }

    q->dim = d;
    q->start = NULL;
    q->finish = NULL;
    return (void*)q;
}

int IntrQ(void *q, void *e) {
    TList aux = calloc(1, sizeof(TCell));

    if (!aux) {
        return -1;
    }

    aux->info = calloc(1, ((TQueue*)q)->dim);

    if (!aux->info) {
        free(aux);
        return -2;
    }
    
    memcpy(aux->info, e, ((TQueue*)q)->dim);

    if (((TQueue*)q)->start == NULL) {
        ((TQueue*)q)->start = aux;
    } else {
        ((TQueue*)q)->finish->next = aux;
    }
    ((TQueue*)q)->finish = aux;
    //printf("hmmmm %s\n", (char*)(aux->info));
    return 1;
}

int Push(void *s, long* e) {
    TList aux = calloc(1, sizeof(TCell));

    if (!aux) {
        printf("pulaaaa\n");
        return -1;
    }

    aux->info = calloc(1, ((TStack*)s)->dim);

    if (!aux->info) {
        printf("pula\n");
        free(aux);
        return -1;
    }

    memcpy(aux->info, e, ((TStack*)s)->dim);

    // ??

    //if (((TStack*)s)->top == NULL) {

    //}
    //((TStack*)s)->top = aux;
    //printf("%ld\n", (long)aux->info);
    //aux = ((TStack*)s)->top->next;
    aux->next = ((TStack*)s)->top;
    ((TStack*)s)->top = aux;

    return 1;
}

int ExtrQ(void *q, void *e) {
    TList aux = NULL;

    if (((TQueue*)q)->start == NULL) {
        printf("n-ar trb sa vezi asta\n");
        return -1;
    }

    aux = ((TQueue*)q)->start;
    
    if (((TQueue*)q)->start == ((TQueue*)q)->finish) {
        //printf("plm?");
        ((TQueue*)q)->finish = NULL;
    }

    ((TQueue*)q)->start = aux->next;
    memcpy(e, aux->info, ((TQueue*)q)->dim);
    //free(aux->info);
    //free(aux);
    return 1;
}

int Pop(void *s, void *e) {
    TList aux = calloc(1, sizeof(TCell));

    if (!aux) {
        return -1;
    }

    aux->info = calloc(1, ((TStack*)s)->dim);

    if (!aux->info) {
        free(aux);
        return -1;
    }

    aux = ((TStack*)s)->top;
    ((TStack*)s)->top = ((TStack*)s)->top->next;
    memcpy(e, aux->info, ((TStack*)s)->dim);
    free(aux->info);
    free(aux);
    return 1;
}