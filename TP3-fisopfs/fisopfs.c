#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fs.h"

superbloque_t superbloque;

#define DEFAULT_FILE_DISK "file.fisopfs"

char *filedisk = DEFAULT_FILE_DISK;

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int i = buscar_inodo(path, &superbloque);
	if (i == -1) {
		fprintf(stderr, "[debug] getattr: %s\n", strerror(errno));
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[i];
	st->st_uid = inode->id_user;
	st->st_gid = inode->id_gid;
	st->st_size = inode->tam;
	st->st_atime = inode->fecha_de_acceso;
	st->st_mtime = inode->fecha_de_modificacion;
	st->st_ctime = inode->fecha_de_creacion;
	st->st_nlink = inode->link_num;
	if (inode->tipo_archivo == REG_T) {
		st->st_mode = S_IFREG | inode->permisos;
	} else if (inode->tipo_archivo == DIR_T) {
		st->st_mode = S_IFDIR | inode->permisos;
	}
	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	int indice = buscar_inodo(path, &superbloque);

	if (indice < 0) {
		fprintf(stderr, "[debug] Error readdir: path no encontrado\n");
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[indice];
	if (inode->tipo_archivo != DIR_T) {
		fprintf(stderr, "[debug] Error readdir: no es un directorio\n");
		return -ENOTDIR;
	}

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int max_entradas = inode->tam / sizeof(entrada_directorio_t);
	entrada_directorio_t *entradas = (entrada_directorio_t *) inode->content;

	for (int i = 0; i < max_entradas; i++) {
		if (entradas[i].inodo != 0) {
			printf("[debug] readdir - entrada: %s (inodo %d)\n",
			       entradas[i].nombre,
			       entradas[i].inodo);
			filler(buffer, entradas[i].nombre, NULL, 0);
		}
	}
	return 0;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	int indice = buscar_inodo(path, &superbloque);

	if (indice < 0) {
		fprintf(stderr, "[debug] Error readdir: path no encontrado\n");
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[indice];
	if (inode->tipo_archivo != REG_T) {
		fprintf(stderr, "[debug] Error readdir: no es un archivo\n");
		return -EISDIR;
	}

	char *content = inode->content;
	size_t file_size = inode->tam;
	if ((size_t) offset >= file_size) {
		return 0;
	}

	if (offset + size > file_size)
		size = file_size - offset;

	memcpy(buffer, content + offset, size);
	inode->fecha_de_acceso = time(NULL);
	return size;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] Inicalizando filesystem.\n");

	FILE *file = fopen(filedisk, "rb");
	if (file == NULL) {
		printf("[debug] No se encontró un filesystem existente, "
		       "creando uno nuevo.\n");
		inicializar_filesystem(&superbloque);
	} else {
		size_t read_size =
		        fread(&superbloque, sizeof(superbloque_t), 1, file);
		if (read_size != 1) {
			fprintf(stderr, "[debug] Error al leer el filesystem.\n");
			fclose(file);
			return NULL;
		}
		fclose(file);
		printf("[debug] Filesystem cargado correctamente.\n");
	}
	return NULL;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s, mode: %o\n", path, mode);

	inodo_t *inode_parent;
	int idx_padre;
	char nombre_archivo[MAX_PATH];

	int err = obtener_dir_padre(
	        path, &superbloque, &inode_parent, &idx_padre, nombre_archivo);
	if (err < 0) {
		return err;
	}

	int hijo = buscar_entrada_en_directorio(&superbloque,
	                                        idx_padre,
	                                        nombre_archivo);
	if (hijo >= 0) {
		fprintf(stderr,
		        "[debug] Error: ya existe hijo '%s'\n",
		        nombre_archivo);
		return -EEXIST;
	}

	int idx = crear_inodo(&superbloque, path, REG_T);
	if (idx < 0) {
		fprintf(stderr,
		        "[debug] create: no hay espacio para crear archivo\n");
		return -ENOSPC;
	}

	inodo_t *inode_child = &superbloque.inodos[idx];
	inode_child->permisos = mode;
	int ok = agregar_entrada_directorio(&superbloque, inode_parent, inode_child);
	if (ok < 0) {
		return -ENOSPC;
	}
	return 0;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy \n");
	FILE *file = fopen(filedisk, "wb");
	if (!file) {
		fprintf(stderr,
		        "[debug] Error abriendo el archivo para guardar: %s\n",
		        strerror(errno));
		return;
	}
	int n = fwrite(&superbloque, sizeof(superbloque_t), 1, file);
	if (n != 1) {
		fprintf(stderr,
		        "[debug] Error guardando el filesystem: %s\n",
		        strerror(errno));
		fclose(file);
		return;
	}
	fflush(file);
	fclose(file);
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);

	fisopfs_destroy(NULL);

	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	inodo_t *inode_parent;
	int idx_padre;
	char nombre_archivo[MAX_PATH];

	int err = obtener_dir_padre(
	        path, &superbloque, &inode_parent, &idx_padre, nombre_archivo);
	if (err < 0) {
		return err;
	}

	int idx = buscar_inodo(path, &superbloque);
	if (idx < 0) {
		fprintf(stderr, "[debug] unlink: archivo no existe\n");
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[idx];
	if (inode->tipo_archivo != REG_T) {
		fprintf(stderr, "[debug] unlink: no es un archivo regular\n");
		return -EISDIR;
	}

	superbloque.bitmap_inodes[idx] = LIBRE;
	memset(inode, 0, sizeof(inodo_t));
	eliminar_entrada_directorio(&superbloque, idx_padre, nombre_archivo);
	return 0;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s - size: %zu - offset: %ld\n",
	       path,
	       size,
	       offset);

	int indice = buscar_inodo(path, &superbloque);
	if (indice < 0) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[indice];
	if (inode->tipo_archivo != REG_T) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		return -EISDIR;
	}

	if (offset + size > MAX_CONTENT) {
		fprintf(stderr,
		        "[debug] Error write: archivo excede tamaño máximo\n");
		return -EFBIG;
	}

	memcpy(inode->content + offset, buffer, size);

	size_t nuevo_tam = offset + size;
	if (nuevo_tam > inode->tam)
		inode->tam = nuevo_tam;

	inode->fecha_de_modificacion = time(NULL);

	return size;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s, size: %ld\n", path, size);

	int idx = buscar_inodo(path, &superbloque);
	if (idx < 0) {
		fprintf(stderr, "[debug] truncate: archivo no existe\n");
		return -ENOENT;
	}

	inodo_t *inode = &superbloque.inodos[idx];
	if (inode->tipo_archivo != REG_T) {
		fprintf(stderr, "[debug] truncate: no es un archivo regular\n");
		return -EISDIR;
	}

	if (size > MAX_CONTENT) {
		fprintf(stderr, "[debug] truncate: tamaño excede máximo\n");
		return -EFBIG;
	}

	if (size < inode->tam) {
		memset(inode->content + size, 0, inode->tam - size);
	}
	inode->tam = size;
	inode->fecha_de_modificacion = time(NULL);
	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] mkdir llamado con path: %s, mode: %o\n", path, mode);
	fflush(stdout);

	inodo_t *inode_parent;
	int padre;
	char nombre_dir[MAX_PATH];

	int err = obtener_dir_padre(
	        path, &superbloque, &inode_parent, &padre, nombre_dir);
	if (err < 0) {
		return err;
	}

	int hijo = buscar_entrada_en_directorio(&superbloque, padre, nombre_dir);
	if (hijo >= 0) {
		fprintf(stderr, "[debug] Error: ya existe hijo '%s'\n", nombre_dir);
		return -EEXIST;
	}

	int idx_hijo = crear_inodo(&superbloque, path, DIR_T);
	if (idx_hijo < 0) {
		fprintf(stderr,
		        "[debug] mkdir: no hay espacio para crear archivo\n");
		return -ENOSPC;
	}
	inodo_t *inode_child = &superbloque.inodos[idx_hijo];
	inode_child->permisos = mode;
	inode_child->tam = 0;
	inode_child->link_num = 2;

	int resp = agregar_entrada_directorio(&superbloque,
	                                      inode_parent,
	                                      inode_child);
	if (resp < 0) {
		fprintf(stderr,
		        "[debug] Error mkdir: %s, No pude agregarlo como "
		        "entrada al directorio padre\n",
		        path);
		return resp;
	}
	// inicializar entradas "." y ".."
	inicializar_entradas_directorio(inode_child, idx_hijo, padre);
	return 0;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	if (strcmp(path, "/") == 0) {
		fprintf(stderr, "[debug] Error rmdir: no se puede eliminar directorio raíz\n");
		return -EBUSY;
	}

	int inodo_a_eliminar = buscar_inodo(path, &superbloque);
	if (inodo_a_eliminar < 0) {
		fprintf(stderr,
		        "[debug] Error rmdir: directorio no encontrado: %s\n",
		        path);
		return -ENOENT;
	}

	inodo_t *dir = &superbloque.inodos[inodo_a_eliminar];
	if (dir->tipo_archivo != DIR_T) {
		fprintf(stderr,
		        "[debug] Error rmdir: no es un directorio: %s\n",
		        path);
		return -ENOTDIR;
	}

	// tengo que ver que el dir este vacio
	// si lo esta lo elimino, sino aviso q no se puede
	if (!directorio_esta_vacio(dir)) {
		fprintf(stderr,
		        "[debug] Error rmdir: directorio no está vacío: %s\n",
		        path);
		return -ENOTEMPTY;
	}

	inodo_t *inode_parent;
	int padre;
	char nombre_dir[MAX_PATH];

	int err = obtener_dir_padre(
	        path, &superbloque, &inode_parent, &padre, nombre_dir);
	if (err < 0) {
		return err;
	}

	if (eliminar_entrada_directorio(&superbloque, padre, nombre_dir) < 0) {
		fprintf(stderr, "[debug] Error rmdir: error eliminando entrada en directorio padre\n");
		return -EIO;
	}
	superbloque.bitmap_inodes[inodo_a_eliminar] = LIBRE;
	memset(dir, 0, sizeof(inodo_t));
	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	int indice = buscar_inodo(path, &superbloque);
	if (indice < 0) {
		printf("[debug] Error utimens - path: %s\n", path);
		return -ENOENT;
	}

	superbloque.inodos[indice].fecha_de_acceso = tv[0].tv_sec;
	superbloque.inodos[indice].fecha_de_modificacion = tv[1].tv_sec;

	return 0;
}

static struct fuse_operations operations = { .getattr = fisopfs_getattr,
	                                     .readdir = fisopfs_readdir,
	                                     .read = fisopfs_read,
	                                     .write = fisopfs_write,
	                                     .destroy = fisopfs_destroy,
	                                     .init = fisopfs_init,
	                                     .flush = fisopfs_flush,
	                                     .create = fisopfs_create,
	                                     .truncate = fisopfs_truncate,
	                                     .unlink = fisopfs_unlink,
	                                     .mkdir = fisopfs_mkdir,
	                                     .rmdir = fisopfs_rmdir,
	                                     .utimens = fisopfs_utimens };

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}
