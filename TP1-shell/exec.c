#include "exec.h"
#include "utils.h"
#include <unistd.h>
#define ERROR -1
#define WRITE 1
#define READ 0
#define MAX_ENV_BUFF 256


// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int indx = block_contains(eargv[i], '=');
		if (indx == -1) {
			printf_debug("ERROR: no es una asignación valida");
			continue;
		}

		char key[MAX_ENV_BUFF];
		char value[MAX_ENV_BUFF];
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, indx);

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);  // solo 3 argumentos
	} else {
		fd = open(file, flags);  // solo 2 argumentos
	}
	if (fd < 0) {
		perror("Error opening file");
	}
	return fd;
}


// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option+
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);

		printf_debug("ERROR: fallo el execvp");
		exit(ERROR);
		break;
	}
	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		printf_debug("ERROR: fallo el execvp de background");
		exit(ERROR);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		r = (struct execcmd *) cmd;
		ejecutar_redirecciones(r);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;
		ejecutar_pipe(p, cmd);
		break;
	}
	}
}

void
ejecutar_pipe(struct pipecmd *p, struct cmd *cmd)
{
	int fd[2];
	if (pipe(fd) == -1) {
		printf_debug("ERROR: no se puede crear el pipe");
		return;
	}

	pid_t pid_izq = fork();
	if (pid_izq == 0) {
		close(fd[READ]);
		int out_fd = dup2(fd[WRITE], STDOUT_FILENO);

		if (out_fd < 0) {
			printf_debug("ERROR: no se puede duplicar un fd");
			exit(ERROR);
		}

		close(fd[WRITE]);
		setpgid(0, 0);
		exec_cmd(p->leftcmd);
		exit(1);
	} else if (pid_izq > 0) {
		close(fd[WRITE]);
		pid_t pid_der = fork();

		if (pid_der == 0) {
			int out_fd = dup2(fd[READ], STDIN_FILENO);

			if (out_fd < 0) {
				printf_debug(
				        "ERROR: no se puede duplicar un fd");
				exit(ERROR);
			}

			close(fd[READ]);
			// struct cmd *right_cmd = parse_line(p->rightcmd->scmd);
			setpgid(0, 0);
			exec_cmd(p->rightcmd);
			exit(1);

		} else if (pid_der > 0) {
			close(fd[WRITE]);
			close(fd[READ]);
			waitpid(pid_izq, NULL, 0);
			waitpid(pid_der, NULL, 0);
			free_command(cmd);
			exit(0);
		} else {
			handle_error(pid_der, cmd);
		}
	} else {
		handle_error(pid_izq, cmd);
	}
}

void
handle_error(int fd[2], struct cmd *cmd)
{
	printf_debug("ERROR: fallo fork");
	close(fd[WRITE]);
	close(fd[READ]);
	free_command(cmd);
	exit(-1);
}

static int
redireccionar(int fd, int fileno)
{
	if (dup2(fd, fileno) == -1) {
		printf_debug("ERROR no se puede duplicar un fd");
		exit(ERROR);
	}
	close(fd);
	return 0;
}


void
ejecutar_redirecciones(struct execcmd *r)
{
	// To check if a redirection has to be performed
	// verify if file name's length (in the execcmd struct)
	// is greater than zero
	// set_environ_vars(r->eargv, r->eargc);
	// int fd;
	if (strlen(r->in_file) > 0) {
		int fd_stdin = open_redir_fd(r->in_file, O_RDONLY);
		if (fd_stdin < 0) {
			printf_debug("No se puede abrir el archivo");
			exit(ERROR);
		}
		redireccionar(fd_stdin, STDIN_FILENO);
	}
	if (strlen(r->out_file) > 0) {
		int fd_stdout =
		        open_redir_fd(r->out_file, O_CREAT | O_WRONLY | O_TRUNC);
		redireccionar(fd_stdout, STDOUT_FILENO);
	}

	if (strlen(r->err_file) > 0) {
		if (strcmp(r->err_file, "&1") == 0) {  // redireccion combinada
			int out_fd = dup2(STDOUT_FILENO, STDERR_FILENO);
			if (out_fd < 0) {
				printf_debug(
				        "ERROR: no se puede duplicar un fd");
				exit(ERROR);
			}
		} else {
			int fd_stderr =
			        open_redir_fd(r->err_file,
			                      O_CREAT | O_WRONLY | O_TRUNC);
			redireccionar(fd_stderr, STDERR_FILENO);
		}
	}
	r->type = EXEC;
	exec_cmd((struct cmd *) r);
}