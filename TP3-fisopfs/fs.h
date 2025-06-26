#ifndef FS_H
#define FS_H

#include <sys/stat.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

#define LIBRE 0
#define OCUPADO 1
#define DIR_T 0  // tipo de archivo directorio
#define REG_T 1  // tipo de archivo regular
#define MAX_PATH 256
#define MAX_CONTENT 4096
#define MAX_INODES 1024
#define ROOT_PATH "/"
#define MAX_DIR_ENTRIES (MAX_CONTENT / sizeof(entrada_directorio_t))
#define S_IFREG 0100000
#define S_IFDIR 0040000

// modos
#define MODE_DIR 0755
#define MODE_FILE 0644

typedef struct inodo {
	int inumero;       // aca si es dir o arch regular
	uid_t id_user;     // id propietario
	gid_t id_gid;      // id grupo
	nlink_t link_num;  // nro de enlaces
	size_t tam;
	mode_t permisos;  // permisos lectura, escritura o ejecutado
	time_t fecha_de_creacion;
	time_t fecha_de_modificacion;
	time_t fecha_de_acceso;
	char path[MAX_PATH];
	char content[MAX_CONTENT];
	char directory_path[MAX_PATH];  // punteros a los bloques de disco
	int tipo_archivo;               // 0 es directorio, 1 es archivo regular
	char nombre[MAX_PATH];          // nombre de la carpeta o archivo
} inodo_t;

typedef struct {
	int inodo;
	char nombre[MAX_PATH];
} entrada_directorio_t;

typedef struct superbloque {
	inodo_t inodos[MAX_INODES];
	int bitmap_inodes[MAX_INODES];
} superbloque_t;

void inicializar_filesystem(superbloque_t *superbloque);

int buscar_inodo(const char *path, superbloque_t *sb);

int obtener_inodo_libre(superbloque_t *sb);

void separar_path(const char *path, char *directory_path, char *nombre);

int crear_inodo(superbloque_t *sb, const char *path, int tipo);

int eliminar_entrada_directorio(superbloque_t *sb,
                                int inodo_padre,
                                const char *nombre);

int directorio_esta_vacio(inodo_t *dir);

int agregar_entrada_directorio(superbloque_t *sb,
                               inodo_t *inodo_parent,
                               inodo_t *inodo_child);

int buscar_entrada_en_directorio(superbloque_t *sb,
                                 int inodo_padre,
                                 const char *nombre);

void crear_entrada(entrada_directorio_t *entrada, const char *nombre, int inodo);

void inicializar_entradas_directorio(inodo_t *inode_dir,
                                     int inodo_self,
                                     int inodo_padre);

int obtener_dir_padre(const char *path,
                      superbloque_t *sb,
                      inodo_t **inode_padre,
                      int *idx_padre,
                      char *nombre_nuevo);

#endif