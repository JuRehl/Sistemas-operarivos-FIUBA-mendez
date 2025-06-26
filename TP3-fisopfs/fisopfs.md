# fisop-fs

Este trabajo implementa **fisopfs**, un sistema de archivos basado en FUSE que permite la creación y manipulación de archivos y directorios de manera jerárquica. Se contemplan operaciones estándar sobre el sistema de archivos, tales como creación, lectura, escritura, renombrado y eliminación, así como el manejo adecuado de metadatos y persistencia en disco.

## Estructuras en Memoria

El sistema de archivos está representado internamente mediante las siguientes estructuras:

### Inodo (`inodo_t`)

Cada archivo o directorio se representa mediante un inodo que contiene los siguientes campos:

- `inumero`: identificador único del inodo.
- `id_user` / `id_gid`: identificadores de usuario y grupo propietario.
- `link_num`: cantidad de enlaces duros al inodo.
- `tam`: tamaño en bytes del contenido del archivo o del directorio.
- `permisos`: permisos de acceso (lectura, escritura, ejecución).
- `fecha_de_creacion`, `fecha_de_modificacion`, `fecha_de_acceso`: fechas relevantes asociadas al archivo o directorio.
- `tipo_archivo`: indica si el inodo representa un archivo regular o un directorio.
- `path`: ruta absoluta del archivo o directorio.
- `directory_path`: ruta del directorio padre.
- `nombre`: nombre del archivo o directorio.
- `content`: contenido del archivo (en caso de archivo regular) o lista serializada de entradas (en caso de directorio).

### Entrada de Directorio (`entrada_directorio_t`)

Cada entrada dentro de un directorio se representa mediante esta estructura, que contiene:

- `inodo`: índice del inodo al que refiere la entrada.
- `nombre`: nombre del archivo o subdirectorio.

Estas entradas están almacenadas en el campo `content` de los inodos que representan directorios.

### Superbloque (`superbloque_t`)

El superbloque mantiene información global del sistema de archivos:

- `inodos`: arreglo estático de inodos con un tamaño máximo definido por `MAX_INODES`.
- `bitmap_inodes`: arreglo que indica si cada inodo se encuentra libre o ocupado, mediante un esquema de bitmap.

El uso de estructuras de tamaño fijo permite simplificar la gestión de memoria y facilita el control de errores relacionados con la capacidad máxima del sistema.

## Funciones para Gestión de Inodos y Directorios

Se implementaron diversas funciones auxiliares para la administración del sistema de archivos:

### Búsqueda y Navegación

- `buscar_inodo(const char *path, superbloque_t *sb)`: busca un inodo según su ruta absoluta.
- `buscar_entrada_en_directorio(...)`: localiza una entrada dentro del contenido de un directorio.
- `obtener_dir_padre(...)`: obtiene el inodo del directorio padre a partir de un path absoluto.
- `separar_path(...)`: separa un path completo en su directorio contenedor y el nombre del archivo o directorio.

### Creación y Eliminación

- `crear_inodo(...)`: inicializa y asigna un nuevo inodo.
- `obtener_inodo_libre(...)`: localiza el primer inodo disponible.
- `crear_entrada(...)`: crea una nueva entrada de directorio.
- `agregar_entrada_directorio(...)`: agrega una entrada a un directorio existente.
- `eliminar_entrada_directorio(...)`: elimina una entrada específica dentro de un directorio.

### Directorios Especiales

- `inicializar_entradas_directorio(...)`: agrega las entradas `"."` y `".."` en un directorio recién creado.
- `directorio_esta_vacio(...)`: verifica si un directorio contiene solo las entradas especiales.

## Consideraciones de Diseño

- Se decidió mantener un sistema de archivos **jerárquico**, en el que cada directorio contiene una lista de entradas (archivos o subdirectorios).
- Los archivos y directorios almacenan su `path` absoluto y su `directory_path`, lo que simplifica las búsquedas y el acceso durante las operaciones de FUSE.
- El contenido de los archivos se guarda directamente en el campo `content`, mientras que los directorios almacenan en el mismo campo las entradas serializadas.
- Se utiliza un esquema de bitmap para la administración del espacio de inodos, permitiendo una búsqueda eficiente de inodos disponibles.
- Todos los límites del sistema (como el tamaño máximo de archivos, cantidad de inodos, etc.) son definidos mediante constantes (`MAX_INODES`, `MAX_CONTENT`, etc.).

## Persistencia en Disco

El sistema de archivos mantiene persistencia mediante las siguientes funciones:

- `fisopfs_init`: se ejecuta al montar el sistema. Intenta abrir el archivo `file.fisopfs` para cargar el contenido del superbloque y sus inodos. Si no existe, se crea un nuevo sistema con un directorio raíz (`/`).
- `fisopfs_destroy`: guarda el estado actual del superbloque en disco al desmontar el sistema, asegurando la persistencia de los cambios.
- `fisopfs_flush`: sincroniza el contenido en memoria con el archivo de respaldo en disco, llamando internamente a `fisopfs_destroy`.

Estas funciones garantizan que las operaciones realizadas por el usuario persistan correctamente entre montajes del sistema.

## Conclusiones

La implementación de este sistema de archivos permitió abordar conceptos fundamentales relacionados con la estructura interna de los sistemas de archivos, la interacción con FUSE, y la administración eficiente de recursos como inodos, directorios y metadatos. El uso de estructuras fijas y un diseño jerárquico simplificado facilitó el desarrollo y la depuración del sistema.

## Ejecución de Pruebas

Para validar el correcto funcionamiento del sistema de archivos, se desarrollaron pruebas automatizadas incluidas en el script `run_test.sh`.

Las pruebas deben ejecutarse dentro de la imagen de Docker de nuestro filesystem. Luego, se debe ubicar en el directorio raíz del proyecto (`fisopfs/`) y ejecutar:

```bash
./run_test.sh
