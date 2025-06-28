#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>    
#include <unistd.h>
#include <errno.h>

// Declaración del arreglo (tamaño arbitrario, más que suficiente para x86-64)
const char* syscall_names[550] = {
#include "syscalls_table.c"
};

const char* nombre_syscall(unsigned long syscall) {
    if (syscall < sizeof(syscall_names)/sizeof(char*) && syscall_names[syscall])
        return syscall_names[syscall];
    return "unknown";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <programa> [args...]\n", argv[0]);
        exit(1);
    }

    pid_t child = fork();

    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);  
        perror("execvp");
        exit(1);
    } else {
        int status;
        struct user_regs_struct regs;
        int in_syscall = 0;

        waitpid(child, &status, 0); 

        while (1) {
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
            waitpid(child, &status, 0);

            if (WIFEXITED(status)) break; 

            ptrace(PTRACE_GETREGS, child, NULL, &regs);

            if (!in_syscall) {
                const char* name = nombre_syscall(regs.orig_rax);
                printf("syscall %s (%llu) = ", name, regs.orig_rax);
                in_syscall = 1;
            } else {
                printf("%llu\n", regs.rax);
                in_syscall = 0;
            }
        }
    }

    return 0;
}
