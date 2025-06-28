// ps_simple.c
#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int es_numero(const char *s) {
    for (int i = 0; s[i]; i++)
        if (!isdigit(s[i])) return 0;
    return 1;
}

int main() {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(proc)) != NULL) {
        if (!es_numero(ent->d_name) && ent->d_type != DT_DIR) continue;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%s/stat", ent->d_name);
        FILE *cmd = fopen(path, "r");
        if (!cmd) continue;

        char buffer[4096];
        size_t len = fread(buffer, 1, sizeof(buffer), cmd);
        fclose(cmd);
        if (len == 0) continue;

        buffer[len] = '\0';

        char nombre[256];
        sscanf(buffer,"%*d (%[^)])",nombre);
        printf("%s\t%s\n", ent->d_name, nombre);
    }

    closedir(proc);
    return 0;
}
