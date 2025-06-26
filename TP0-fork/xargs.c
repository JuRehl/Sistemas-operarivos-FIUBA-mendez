#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

void xargs_auxiliar(char *comando, int i, char *buffer[NARGS + 2]);

int
main(int argc, char *argv[])
{
	char *comando = argv[1];
	char *buffer[NARGS + 2];

	ssize_t lineas;
	char *linea = NULL;
	size_t tamaño = 0;
	int i = 0;

	while ((lineas = getline(&linea, &tamaño, stdin)) != -1) {
		linea[lineas - 1] = '\0';
		buffer[i + 1] = strdup(linea);
		i++;

		if (i == NARGS) {
			xargs_auxiliar(comando, i, buffer);
			i = 0;
		}
	}
	if (i > 0) {
		xargs_auxiliar(comando, i, buffer);
	}
	free(linea);
	return 0;
}
void
xargs_auxiliar(char *comando, int i, char *buffer[NARGS + 2])
{
	buffer[0] = comando;
	buffer[i + 1] = NULL;
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		execvp(comando, buffer);
		perror("execvp");
		exit(1);
	} else {
		wait(NULL);
	}

	for (int j = 1; j <= i; j++) {
		free(buffer[j]);
	}
}