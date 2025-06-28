#include <inc/lib.h>

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    envid_t env_id = sys_getenvid();

    int initial_priority = sys_obtener_prioridad(env_id);
    cprintf("Env %08x inicia con prioridad %d\n", env_id, initial_priority);

    for (int i = 0; i < 5; i++) {
        sys_modificar_prioridad();
        int current_priority = sys_obtener_prioridad(env_id);
        cprintf("Env %08x se ejecuta con prioridad %d (iteración %d)\n", env_id, current_priority, i + 1);
        sys_yield();
    }

    int final_priority = sys_obtener_prioridad(env_id);
    cprintf("Env %08x termina con prioridad %d\n", env_id, final_priority);
}


