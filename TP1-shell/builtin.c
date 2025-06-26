#include "builtin.h"
#include "defs.h"
#include "utils.h"
#include <unistd.h>
#include "string.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strncmp(cmd, "exit", 4) != 0) {
		return 0;
	}
	exit(1);
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char buf[BUFLEN];
	if (strcmp(cmd, "cd") == 0) {
		char *path = getenv("HOME");
		if (path == NULL) {
			perror("error al buscar variable de entorno en cd");
			return 0;
		}
		if (chdir(path) < 0) {
			snprintf(buf, sizeof buf, "cannot cd to %s", path);
			perror(buf);
			return 0;
		}
		snprintf(prompt, sizeof prompt, "(%s)", getcwd(buf, PRMTLEN));
		return 1;
	} else if (strncmp(cmd, "cd ", 3) == 0) {
		cmd = cmd + 3;
		if (chdir(cmd) < 0) {
			snprintf(buf, sizeof buf, "cannot cd to (%s) ", cmd);
			perror(buf);
			return 0;
		}
		snprintf(prompt, sizeof prompt, "(%s)", getcwd(buf, PRMTLEN));
		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char *cwd = getcwd(NULL, 0);
		if (!cwd) {
			printf_debug("ERROR: fallo en pwd");
		} else {
			printf("%s\n", cwd);
			free(cwd);
			return 1;
		}
	}
	return 0;
}
// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
