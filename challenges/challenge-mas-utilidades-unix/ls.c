// ls.c
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

char tipo_char(mode_t m) {
    if (S_ISREG(m)) return '-';
    if (S_ISDIR(m)) return 'd';
    if (S_ISLNK(m)) return 'l';
    if (S_ISCHR(m)) return 'c';
    if (S_ISBLK(m)) return 'b';
    if (S_ISFIFO(m)) return 'p';
    if (S_ISSOCK(m)) return 's';
    return '?';
}

void mostrar_permisos(mode_t m) {
    printf("%o", m & 0777);
}

int main(int argc, char *argv[]) {
    const char *path = argc > 1 ? argv[1] : ".";

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        char ruta[1024];
        snprintf(ruta, sizeof(ruta), "%s/%s", path, ent->d_name);

        struct stat st;
        if (lstat(ruta, &st) == -1) {
            perror("lstat");
            continue;
        }

        printf("%s", ent->d_name);
        printf("%c ", tipo_char(st.st_mode));
        mostrar_permisos(st.st_mode);
        printf(" %d",st.st_uid);
        

        if (S_ISLNK(st.st_mode)) {
            char enlace[1024];
            ssize_t len = readlink(ruta, enlace, sizeof(enlace) - 1);
            if (len != -1) {
                enlace[len] = '\0';
                printf(" -> %s", enlace);
            }
        }

        printf("\n");
    }

    closedir(dir);
    return 0;
}
