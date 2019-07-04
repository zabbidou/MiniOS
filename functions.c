// Gherman Maria Irina @ 314CB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "struct_functions.c"

// functie pentru a gasi adresa unui proces in lista de memorie
// dupa pid
TProcess find_process(TProcess c_process, TQueue* w, TQueue* f, int pid) {
    TQueue* waux = InitQ(w->dim);
    if (!waux) {
        return NULL;
    }
    
    TQueue* faux = InitQ(f->dim);
    if (!faux) {
        return NULL;
    }

    TProcess paux1 = calloc(1, sizeof(TProc));
    if (!paux1) {
        return NULL;
    }
    
    TProcess paux2 = calloc(1, sizeof(TProc));
    if (!paux2) {
        free(paux1);
        return NULL;
    }

    TProcess good = calloc(1, sizeof(TProc));
    if (!good) {
        free(paux1);
        free(paux2);
        return NULL;
    }

    int found = 0;
    int i = 0;

    if (c_process == NULL) {
        return NULL;
    }
    // daca este procesul curent
    if (c_process->pid == pid) {
        return c_process;
    }
    // cautam in waiting
    while (w->start != NULL) {
        ExtrQ(w, paux1);
        i++;

        if (paux1->pid == pid) {
            memcpy(good, paux1, sizeof(TProc));
            found = 1;
        }

        IntrQ(waux, paux1);
    }
    // punem waiting la loc
    while (waux->start != NULL) {
        ExtrQ(waux, paux1);
        IntrQ(w, paux1);
    }

    if (found == 1) {
        return good;
    }
    // cautam in finished
    i = 0;
    while (f->start != NULL) {
        ExtrQ(f, paux2);
        if (paux2->pid == pid) {
            found = 1;
            memcpy(good, paux2, sizeof(TProc));
        }

        IntrQ(faux, paux2);
    }
    // punem finished la loc
    while (faux->start != NULL) {
        ExtrQ(faux, paux2);
        IntrQ(f, paux2);
    }

    if (found == 1) {
        return good;
    }

    return NULL;
}
// gaseste prima adresa valabila
long GetMemStart(TMemList L, TProcess proc) {
    if (L->empty_list == 1) {
        // daca e primul proces inserat, inceputul e 0x0
        return 0;
    }

    if (L->next == NULL) {
        // daca e al doilea proces, inceputul e sfarsitul primuluo
        return ((TProcess)L->info)->memsize;
    }

    if (((TProcess)L->info)->memstart >= proc->memsize) {
        // daca are loc inaintea primului proces din lista
        return 0;
    }

    while (L != NULL) {
        if (L->prev != NULL) {
            if (L->next == NULL) {
                if (((TProcess)L->info)->memstart + 
                    ((TProcess)L->info)->memsize + 
                    proc->memsize > 3 * 1024 * 1024) {
                    return -1;
                }

                return ((TProcess)L->info)->memstart + 
                       ((TProcess)L->info)->memsize;
            } else {
                if (((TProcess)L->info)->memstart - 
                   ((TProcess)L->prev->info)->memstart - 
                   ((TProcess)L->prev->info)->memsize >= proc->memsize) {
                    // cazul de inserare la mijloc, dar nu am ajuns la 
                    // testul care il foloseste
                    // asa ca il las comentat

                    // return ((TProcess)L->prev->info)->memstart + 
                    //        ((TProcess)L->prev->info)->memsize;
                }
            }
        }
        L = L->next;
    }
    return -1;
}
// cauta primul pid disponibil
int GetPID(TMemList L, char pid_list[32768]) {
    int i = 1;
    while (i < 32768) {
        if (pid_list[i] == 0) {
            pid_list[i] = 1;
            break;
        }
        i++;
    }
    return i;
}
// baga procesul in lista de memorie
TMemList alloc_memory(TMemList *L, TProcess proc, long memstart) {
    TMemList p = *L;
    TMemList aux = calloc(1, sizeof(TMemCell));

    if (!aux) {
        return NULL;
    }
    
    aux->next = NULL;
    aux->prev = NULL;
    aux->info = calloc(1, sizeof(TProc));

    if (!aux->info) {
        free(aux);
        return NULL;
    }

    if ((*L)->info == NULL) {
        (*L)->info = calloc(1, sizeof(TProc));
        memcpy((*L)->info, proc, sizeof(TProc));
        (*L)->empty_list = 0;
        (*L)->next = NULL;
        return *L;
    }

    if (memstart == 0) {
        aux->next = (*L);
        (*L)->prev = aux;
        (*L) = aux;
        aux->prev = NULL;

        return *L;
    }

    while (p->next != NULL && 
        ((TProcess)p->info)->memstart + 
        ((TProcess)p->info)->memsize != memstart) {

        p = p->next;
    }

    aux->prev = p;

    if (p->next != NULL) {
        p->next->prev = aux;
        aux->next = p->next;
    } else {
        p->next = aux;
    }

    memcpy(aux->info, proc, sizeof(TProc));
    (*L)->empty_list = 0;
    return (*L);
}
// contruieste procesul propriu zis
TProcess build_process(TMemList *L, long memsize, long time, 
                       int priority, char pid_list[32768], 
                       FILE* out, long T) {

    TProcess process = malloc(sizeof(TProc));
    if (!process) {
        return NULL;
    }

    process->memstack = InitS(sizeof(long));

    process->memsize = memsize;
    process->time = time;
    process->time_remaining = time;
    process->cuanta_ramasa = T;
    process->priority = priority;
    process->pid = GetPID(*L, pid_list);
    process->memstart = GetMemStart(*L, process);

    if (process->memstart == -1) {
        fprintf(out, "Cannot reserve memory for PID %d.\n", process->pid);
        pid_list[process->pid] = 0;
        free(process->memstack);
        free(process);
        return NULL;
    }

    *L = alloc_memory(L, process, process->memstart);
    
    return process;
}
// afiseaza cozile de procese
void PrintQueue(TQueue *q, FILE* out, char string[15]) {
    TProcess aux = malloc(sizeof(TProc));
    TQueue* qaux = InitQ(q->dim);

    if (!q) {
        printf("[plm?]\n");
        return;
    }

    if (!q->start) {
        fprintf(out, "[]\n");        
        return;
    }
        
    ExtrQ(q, aux);

    fprintf(out, "[(%d: priority = %d, %s = %ld)", 
            aux->pid, aux->priority, string, aux->time_remaining);

    IntrQ(qaux, aux);

    while (q->start != NULL) {
        ExtrQ(q, aux);

        fprintf(out, ",\n(%d: priority = %d, %s = %ld)", 
                aux->pid, aux->priority, string, aux->time_remaining);

        IntrQ(qaux, aux);
    }
    
    fprintf(out, "]\n");
    
    while (qaux->start != NULL) {
        ExtrQ(qaux, aux);
        IntrQ(q, aux);
    }

    free(qaux);
}

void InsOrd(TQueue *q, TProcess p) {
    TProcess aux = malloc(sizeof(TProc));
    TQueue *qaux = InitQ(q->dim);
    int inserted = 0;

    while (q->start != NULL) {
        ExtrQ(q, aux);
        if (inserted == 0) {
            if (aux->priority < p->priority) {
                IntrQ(qaux, p);
                IntrQ(qaux, aux);
                inserted = 1;
                continue;
            }

            if (aux->priority == p->priority) {
                if (aux->time_remaining > p->time_remaining) {

                    IntrQ(qaux, p);
                    IntrQ(qaux, aux);
                    inserted = 1;
                    continue;
                }
            
                if (aux->time_remaining == p->time_remaining) {
                    if (aux->pid > p->pid) {
                        IntrQ(qaux, p);
                        IntrQ(qaux, aux);
                        inserted = 1;
                        continue;
                    } else {
                        IntrQ(qaux, aux);
                        continue;
                    }
                } else {
                    IntrQ(qaux, aux);
                    continue;
                }
            }
        }

        IntrQ(qaux, aux);
    }

    if (inserted == 0) {
        IntrQ(qaux, p);
    }

    while (qaux->start != NULL) {
        ExtrQ(qaux, aux);
        IntrQ(q, aux);
    }
}
// adauga in coada de waiting
void add_waiting(TQueue *wait, TProcess proc) {
    if (wait->start == NULL) {
        IntrQ(wait, proc);
    } else {
        InsOrd(wait, proc);
    }
}

void get(int pid, TProcess current_process, TQueue *w, TQueue *f, FILE* out) {
    TQueue *qaux = InitQ(w->dim);
    TProcess p = malloc(sizeof(TProc));
    int found = 0;

    if (current_process->pid == pid) {
        fprintf(out, "Process %d is running (remaining_time: %ld).\n",
               pid, current_process->time_remaining);
        found = 1;
    } else {
        while (w->start != NULL) {
            ExtrQ(w, p);
            
            if (p->pid == pid) {
                fprintf(out, "Process %d is waiting (remaining_time: %ld).\n",
                        pid, p->time_remaining);
                found = 1;
            }
            
            IntrQ(qaux, p);
        }

        while (qaux->start != NULL) {
            ExtrQ(qaux, p);
            IntrQ(w, p);
        }
    }
    
    if (found == 0) {
        while (f->start != NULL) {
            ExtrQ(f, p);
            
            if (p->pid == pid) {
                fprintf(out, "Process %d is finished.\n", pid);
                found = 1;
            }
            
            IntrQ(qaux, p);
        }

        while (qaux->start != NULL) {
            ExtrQ(qaux, p);
            IntrQ(f, p);
        }
    }

    if (found == 0) {
        fprintf(out, "Process %d not found.\n", pid);
    }
    return;
}

int free_memory(TMemList *L, TProcess proc) {
    TMemList p = *L;
    int found = 0;

    if ((*L) == NULL) {
        return -2;
    }

    while (p != NULL) {
        if (((TProcess)p->info)->pid == proc->pid) {
            found = 1;
            break;
        }
        p = p->next;
    }
    
    TMemList aux = p;

    if (p->prev == NULL && p->next == NULL) {
        (*L)->empty_list = 1;
    }

    if ((*L)->next == p && found == 0) {
        (*L)->empty_list = 1;
    }

    if (found == 1) {
        if (p->prev != NULL) {
            p->prev->next = p->next;
        } else {
            (*L) = (*L)->next;
        }

        if (p->next != NULL) {
            p->next->prev = p->prev;
        } else {
            p->prev->next = NULL; 
        }
    }

    aux->prev = NULL;
    aux->next = NULL;

    free(aux);

    return 1;
}
// functia asta e incompleta pentru ca nu am ajuns la cazurile acelea, dar las
// codul comentat pentru ca vreau sa se vada ca am gandit-o
TProcess run(TMemList *L, TProcess c_process, TQueue* w, TQueue* f, long t, 
             long T, char pid_list[32768]) {
    // t-> timpul cat sa ruleze
    // T-> cuanta
    // w-> waiting queue
    // f-> finished queue
    // c_process-> current process
    long T_r = T; // partea din cuanta ramasa dupa rularea unui proces
    int cuanta_schimbata = 0;
    int switch_p = 0;
    long time = T;

    TProcess new_c_process = malloc(sizeof(TProc));
    if (!new_c_process) {
        return NULL;
    }

    while (t > 0) {
        if (c_process == NULL && w->start == NULL) {
            break;
        }

        /*
        if (fuckery == 1) {
            cuanta_schimbata = 0;
            T = T_r;
        } else {
            T = time;
        }*/

        if (t < T) {
            T = t;
        }

        if (c_process->time_remaining < T && c_process->time_remaining != 0) {
            // daca ruleaza mai putin decat cuanta (ex: c:500, 
                                                // ruleaza doar 50)
            T_r = T - c_process->time_remaining;
            c_process->time_remaining = 0;
            cuanta_schimbata = 1;
            switch_p = 1;
        }

        if (c_process->time_remaining >= T) {
            // daca sunt in cazul de baza
            if (c_process->cuanta_ramasa == time && T == time) {
                // cazul de baza
                c_process->time_remaining = c_process->time_remaining - T;
                switch_p = 1;
                t = t - T;
            }
            // daca procesul meu mai trebuie sa ruleze o parte din cuanta
            // cazul asta trebuia completat dar cum nu am ajuns la testele
            // care au nevoie de asta, il las comentat
            if (c_process->cuanta_ramasa < T) {
                //T_r = T - c_process->cuanta_ramasa;
                //cuanta_schimbata = 1;
                switch_p = 1;
                t = t - c_process->cuanta_ramasa;
                c_process->time_remaining = c_process->time_remaining - 
                                            c_process->cuanta_ramasa;
                c_process->cuanta_ramasa = time;
            }

            // daca am un fragment de cuanta ramasa de la celalalt proces
            if (T < time) {
                c_process->time_remaining = c_process->time_remaining - T;
                cuanta_schimbata = 0;
                c_process->cuanta_ramasa = c_process->cuanta_ramasa - T;
                t = t - T;
            } 
        }
        // procesul e finished
        if (c_process->time_remaining == 0) {

            c_process->time_remaining = c_process->time;
            IntrQ(f, c_process);
            pid_list[c_process->pid] = 0;
            free_memory(L, c_process);
            ExtrQ(w, c_process);
            continue;
        }

        if (t < 0) {
            t = 0;
            break;
        }
        // trebuie schimbat procesul din running
        if (w->start != NULL && switch_p == 1) {
            ExtrQ(w, new_c_process);
            add_waiting(w, c_process);
            memcpy(c_process, new_c_process, sizeof(TProc));
        }
    }

    free(new_c_process);

    return c_process;
}
// afiseaza stiva procesului
int print_stack(TProcess proc, FILE* out) {
    if (!proc) {
        return -2;
    }

    TStack* s = proc->memstack;
    TStack* saux = InitS(s->dim);
    long* aux = calloc(1, sizeof(long));

    if (!aux) {
        return -2;
    }

    if (!s) {
        return -1;
    }

    if (s->top != NULL) {
        fprintf(out, "Stack of PID %d: ", proc->pid);
    } else {
        fprintf(out, "Empty stack PID %d.\n", proc->pid);
        return 0;
    }

    while (s->top != NULL) {
        Pop(s, aux);
        fprintf(out, "%ld", *aux);
        Push(saux, aux);
    }

    fprintf(out, ".\n");

    while(saux->top != NULL) {
        Pop(saux, aux);
        Push(s, aux);
    }

    free(aux);

    return 1;
}
// introduce in stiva procesului
int push_for_process(TProcess proc, long* info, FILE* out) {
    if (proc == NULL) {
        return 0;
    }

    Push(proc->memstack, info);

    return 1;
}
// scoate din stiva procesului
int pop_for_process(TProcess proc, FILE* out) {
    if (!proc) {
        return -1;
    }

    TStack* s = proc->memstack;
    long* info = calloc(1, sizeof(long));
    if (!info) {
        return -2;
    }

    if (!s->top) {
        fprintf(out, "Empty stack PID %d.\n", proc->pid);
        return 0;
    }

    Pop(s, info);

    free(info);

    return 1;
}
// comanda finish
long finish_running(TMemList L, TProcess current_process, TQueue* w) {
    long time = 0;
    TProcess aux = calloc(1, sizeof(TProc));
    if (!aux) {
        return -2;
    }

    if (current_process != NULL) {
        time = time + current_process->time_remaining;
    }

    while (w->start != NULL) {
        ExtrQ(w, aux);
        time = time + aux->time_remaining;
    }

    return time;

}