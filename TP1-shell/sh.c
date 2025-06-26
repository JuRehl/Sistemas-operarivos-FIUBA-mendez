#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"


char prompt[PRMTLEN] = { 0 };

void
sigchild_handler(int signum)
{
	pid_t pid;
	int estado;
	while ((pid = waitpid(0, &estado, WNOHANG)) > 0) {
		char str[BUFLEN] = { 0 };
		snprintf(str, sizeof str, "==> terminado: PID=%d\n", pid);
		write(STDOUT_FILENO, str, strlen(str));
	}
}


// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	stack_t ss;
	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}

	struct sigaction sa;
	sa.sa_handler = sigchild_handler;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction error");
		exit(1);
	}

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
