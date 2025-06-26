#include "fs.h"

void
inicializar_inodos(superbloque_t *superbloque)
{
	// Marcar todos los inodos como libres
	for (int i = 0; i < MAX_INODES; i++) {
		superbloque->bitmap_inodes[i] = LIBRE;
		memset(&superbloque->inodos[i], 0, sizeof(inodo_t));
	}
}

void
inicializar_filesystem(superbloque_t *superbloque)
{
	inicializar_inodos(superbloque);

	// creacion del directorio raiz
	inodo_t *root = &superbloque->inodos[0];
	root->inumero = 0;
	root->tam = 0;
	root->id_user = geteuid();
	root->id_gid = getegid();
	root->permisos = MODE_DIR;
	root->link_num = 2;
	root->fecha_de_creacion = time(NULL);
	root->fecha_de_modificacion = time(NULL);
	root->fecha_de_acceso = time(NULL);
	strcpy(root->path, ROOT_PATH);
	memset(root->content, 0, sizeof(root->content));
	root->tipo_archivo = DIR_T;
	strcpy(root->directory_path, "");
	superbloque->bitmap_inodes[0] = OCUPADO;
}

// Busca el índice del inodo cuyo path coincida exactamente con 'path'.
// Devuelve índice si lo encuentra, o -1 si no existe.
int
buscar_inodo(const char *path, superbloque_t *sb)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (sb->bitmap_inodes[i] == OCUPADO &&
		    strcmp(sb->inodos[i].path, path) == 0) {
			return i;
		}
	}
	return -1;
}

// Busca un índice libre en el bitmap de inodos.
// Devuelve el índice disponible o -1 si no hay espacio.
int
obtener_inodo_libre(superbloque_t *sb)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (sb->bitmap_inodes[i] == LIBRE) {
			return i;
		}
	}
	return -1;
}

void
separar_path(const char *path, char *directory_path, char *nombre)
{
	const char *last_slash = strrchr(path, '/');

	if (!last_slash) {
		// No tiene '/' --> raíz y nombre es todo el path
		strcpy(directory_path, ROOT_PATH);
		strncpy(nombre, path, MAX_PATH - 1);
		nombre[MAX_PATH - 1] = '\0';
	} else if (last_slash == path) {
		// Está en raíz, ej: "/archivo"
		strcpy(directory_path, ROOT_PATH);
		strncpy(nombre, last_slash + 1, MAX_PATH - 1);
		nombre[MAX_PATH - 1] = '\0';
	} else {
		// Directorio padre es todo hasta el último '/'
		size_t dir_len = last_slash - path;
		if (dir_len >= MAX_PATH)
			dir_len = MAX_PATH - 1;

		strncpy(directory_path, path, dir_len);
		directory_path[dir_len] = '\0';

		// Nombre es lo que queda después del último '/'
		strncpy(nombre, last_slash + 1, MAX_PATH - 1);
		nombre[MAX_PATH - 1] = '\0';
	}
}

int
crear_inodo(superbloque_t *sb, const char *path, int tipo)
{
	int i = obtener_inodo_libre(sb);
	if (i == -1) {
		return -1;
	}

	inodo_t *inode = &sb->inodos[i];
	sb->bitmap_inodes[i] = OCUPADO;

	// Inicializar campos
	inode->inumero = i;
	inode->tam = 0;
	inode->id_user = geteuid();
	inode->id_gid = getegid();
	inode->link_num = 1;
	inode->tipo_archivo = tipo;
	inode->permisos = (tipo == DIR_T) ? MODE_DIR : MODE_FILE;

	time_t ahora = time(NULL);
	inode->fecha_de_creacion = ahora;
	inode->fecha_de_modificacion = ahora;
	inode->fecha_de_acceso = ahora;

	memset(inode->content, 0, sizeof(inode->content));

	strncpy(inode->path, path, MAX_PATH - 1);
	inode->path[MAX_PATH - 1] = '\0';

	separar_path(path, inode->directory_path, inode->nombre);

	return i;
}

int
eliminar_entrada_directorio(superbloque_t *sb, int inodo_padre, const char *nombre)
{
	inodo_t *padre = &sb->inodos[inodo_padre];
	entrada_directorio_t *entradas = (entrada_directorio_t *) padre->content;
	int n_entradas = padre->tam / sizeof(entrada_directorio_t);

	for (int i = 0; i < n_entradas; i++) {
		if (entradas[i].inodo != 0 &&
		    strcmp(entradas[i].nombre, nombre) == 0) {
			// mover entradas posteriores para mantener las entradas contiguas
			for (int j = i; j < n_entradas - 1; j++) {
				entradas[j] = entradas[j + 1];
			}
			entradas[n_entradas - 1].inodo = 0;
			entradas[n_entradas - 1].nombre[0] = '\0';
			padre->tam -= sizeof(entrada_directorio_t);
			return 0;
		}
	}

	return -1;  // No se encontró
}

int
directorio_esta_vacio(inodo_t *dir)
{
	entrada_directorio_t *entradas = (entrada_directorio_t *) dir->content;

	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (entradas[i].inodo != 0 &&
		    strcmp(entradas[i].nombre, ".") != 0 &&
		    strcmp(entradas[i].nombre, "..") != 0) {
			return 0;  // No está vacío
		}
	}

	return 1;  // Vacío
}

int
agregar_entrada_directorio(superbloque_t *sb,
                           inodo_t *inodo_parent,
                           inodo_t *inodo_child)
{
	entrada_directorio_t *entradas =
	        (entrada_directorio_t *) inodo_parent->content;

	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (entradas[i].inodo == 0) {  // Suponiendo 0 = vacío
			entradas[i].inodo = inodo_child->inumero;
			strncpy(entradas[i].nombre,
			        sb->inodos[inodo_child->inumero].nombre,
			        MAX_PATH - 1);
			entradas[i].nombre[MAX_PATH - 1] = '\0';
			inodo_parent->tam += sizeof(entrada_directorio_t);
			return 0;
		}
	}

	return -1;  // No hay lugar en el directorio
}

void
crear_entrada(entrada_directorio_t *entrada, const char *nombre, int inodo)
{
	strcpy(entrada->nombre, nombre);
	entrada->inodo = inodo;
}

void
inicializar_entradas_directorio(inodo_t *inode_dir, int nuevo_inodo, int inodo_padre)
{
	entrada_directorio_t *entradas =
	        (entrada_directorio_t *) inode_dir->content;
	crear_entrada(&entradas[0], ".", nuevo_inodo);
	crear_entrada(&entradas[1], "..", inodo_padre);
	inode_dir->tam = 2 * sizeof(entrada_directorio_t);
}

int
buscar_entrada_en_directorio(superbloque_t *sb, int inodo_padre, const char *nombre)
{
	inodo_t *padre = &sb->inodos[inodo_padre];
	entrada_directorio_t *entradas = (entrada_directorio_t *) padre->content;

	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (entradas[i].inodo != 0 &&
		    strcmp(entradas[i].nombre, nombre) == 0) {
			return entradas[i].inodo;
		}
	}
	return -1;
}

int
obtener_dir_padre(const char *path,
                  superbloque_t *sb,
                  inodo_t **inode_padre,
                  int *idx_padre,
                  char *nombre_nuevo)
{
	char path_padre[MAX_PATH];
	separar_path(path, path_padre, nombre_nuevo);

	int padre = buscar_inodo(path_padre, sb);
	if (padre < 0) {
		fprintf(stderr, "[debug] Error: no existe el directorio padre\n");
		return -ENOENT;
	}

	inodo_t *inode_parent = &sb->inodos[padre];
	if (inode_parent->tipo_archivo != DIR_T) {
		fprintf(stderr, "[debug] Error: el padre no es un directorio\n");
		return -ENOTDIR;
	}

	*inode_padre = inode_parent;
	*idx_padre = padre;
	return 0;
}
