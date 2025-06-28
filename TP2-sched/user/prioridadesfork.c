#include <inc/lib.h>

#include <inc/lib.h>

// Imprimir su prioridad inicial
void print_env_start(void) {
    int priority = sys_obtener_prioridad(sys_getenvid());
    cprintf("Env %08x starts with priority %d\n", sys_getenvid(), priority);
}

// Imprimir prioridad final
void print_env_end(void) {
    int priority = sys_obtener_prioridad(sys_getenvid());
    cprintf("Env %08x ends with priority %d\n", sys_getenvid(), priority);
}

void get_priority_and_yield_in_loop(int executions) {
    for (int i = 0; i < executions; i++) {
        int priority = sys_obtener_prioridad(sys_getenvid());
        cprintf("Env %08x is paused with priority %d\n", sys_getenvid(), priority);
        sys_yield();
    }
}

void run_env_behavior(void) {
    print_env_start();
    sys_modificar_prioridad(); // modificar prioridad
    get_priority_and_yield_in_loop(5);
    print_env_end();
}

void umain(int argc, char **argv) {
    int id = fork();
    if (id < 0)
        panic("fork: %e", id);

    if (id == 0) {
        int id2 = fork();
        if (id2 < 0)
            panic("fork: %e", id2);

        if (id2 == 0) {
            run_env_behavior();
        } else {
            run_env_behavior();
        }
    } else {
        run_env_behavior();
    }
}

