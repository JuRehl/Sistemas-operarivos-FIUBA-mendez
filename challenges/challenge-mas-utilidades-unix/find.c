#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

void buscar(const char *dir_path, const char *buscado) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Error al abrir directorio");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (strstr(entry->d_name, buscado) != NULL) {
            printf("%s\n", full_path);
        }

        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            buscar(full_path, buscado);
        }
    }

    closedir(dir);
}

void a_minuscula(char* cadena) {
    for (int i = 0; cadena[i] != '\0'; i++) {
        cadena[i] = tolower(cadena[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc !=3) {
        fprintf(stderr, "Uso: %s cadena_a_buscar\n", argv[0]);
        return 1;
    }

    char *buscado = argv[1];
    if (argc == 3){
        buscado=argv[2];
        a_minuscula(buscado);
    }
    buscar(".", buscado);
    return 0;
}
