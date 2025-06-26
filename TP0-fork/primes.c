#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void filtro(int pipe_padre[2]);

int
main(int argc, char *argv[])
{
	int n = atoi(argv[1]);

	int pipe_padre[2];
	if (pipe(pipe_padre) == -1) {
		perror("pipe");
		close(pipe_padre[0]);
		close(pipe_padre[1]);
		exit(1);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		close(pipe_padre[1]);
		filtro(pipe_padre);
		close(pipe_padre[0]);
	} else {
		close(pipe_padre[0]);
		for (int i = 2; i <= n; i++) {
			write(pipe_padre[1], &i, sizeof(i));
		}
		close(pipe_padre[1]);
		wait(NULL);
	}
	return 0;
}

void
filtro(int pipe_padre[2])
{
	int recibido;
	ssize_t leido = read(pipe_padre[0], &recibido, sizeof(int));
	if (leido == -1) {
		perror("read");
		close(pipe_padre[0]);
		exit(1);
	}
	if (leido == 0) {
		close(pipe_padre[0]);
		return;
	}

	printf("primo %d\n", recibido);

	int pipe_hijo[2];
	if (pipe(pipe_hijo) == -1) {
		perror("pipe");
		close(pipe_padre[0]);
		exit(1);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		close(pipe_padre[0]);
		close(pipe_hijo[1]);
		filtro(pipe_hijo);
		close(pipe_hijo[0]);
	} else {
		close(pipe_hijo[0]);
		int numero;
		while (read(pipe_padre[0], &numero, sizeof(int)) > 0) {
			if (numero % recibido != 0) {
				write(pipe_hijo[1], &numero, sizeof(int));
			}
		}
		close(pipe_hijo[1]);
		close(pipe_padre[0]);
		wait(NULL);
	}
}
