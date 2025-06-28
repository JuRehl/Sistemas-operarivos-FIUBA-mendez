#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s archivo_origen archivo_destino\n", argv[0]);
        return 1;
    }

    char *src = argv[1];
    char *dst = argv[2];

    int fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        perror("Error al abrir archivo fuente");
        return 1;
    }
    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd_dst < 0) {
        perror("Error al crear archivo destino (ya existe o error)");
        close(fd_src);
        return 1;
    }

    char buf[BUF_SIZE];
    ssize_t leidos;

    while ((leidos = read(fd_src, buf, BUF_SIZE)) > 0) {
        ssize_t escritos = 0;
        while (escritos < leidos) {
            ssize_t w = write(fd_dst, buf + escritos, leidos - escritos);
            if (w < 0) {
                perror("Error al escribir en archivo destino");
                close(fd_src);
                close(fd_dst);
                return 1;
            }
            escritos += w;
        }
    }

    if (leidos < 0) {
        perror("Error al leer archivo fuente");
    }

    close(fd_src);
    close(fd_dst);
    return 0;
}
