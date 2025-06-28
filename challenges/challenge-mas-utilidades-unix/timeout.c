#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

pid_t pid = 0;

struct sigevent event = {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo = SIGALRM,
};

void kill_process(__attribute__((unused)) int sig) {
    if (pid > 0) {
        kill(pid, SIGTERM);
    }
}

void create_action(void) {
    struct sigaction action;
    action.sa_handler = &kill_process;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);
}

void create_event(int duration) {
    struct itimerspec timer;
    timer.it_value.tv_sec = duration;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;

    timer_t timer_id;
    if (timer_create(CLOCK_REALTIME, &event, &timer_id) == -1) {
        perror("timer_create");
        exit(1);
    }

    if (timer_settime(timer_id, 0, &timer, NULL) == -1) {
        perror("timer_settime");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    int duration;

    if (argc < 3) {
        errno = EINVAL;
        perror("Número inválido de argumentos");
        return 1;
    }

    duration = strtol(argv[1], NULL, 10);
    if (duration <= 0) {
        errno = EINVAL;
        perror("Duración inválida");
        return 1;
    }

    create_action();
    create_event(duration);

    pid = fork();
    if (pid < 0) {
        perror("fork falló");
        return 1;
    } else if (pid == 0) {
        execvp(argv[2], argv + 2);
        perror("execvp falló");
        return 1;
    }

    wait(NULL);

    return 0;
}
