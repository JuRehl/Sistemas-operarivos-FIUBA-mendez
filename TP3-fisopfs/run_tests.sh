
function check_operation {
    if [ $? -eq 0 ]; then
         echo "$1: OK"
     else
         echo "$1: FAILED"
     fi
 }

rm -rf prueba

sleep 1

mkdir prueba

./fisopfs ./prueba
sleep 1
mount | grep fisopfs
sleep 1

ls -al
check_operation "Listar contenido del directorio"

echo "Creando archivo test.txt con touch..."
touch test.txt
check_operation "Crear archivo test.txt con touch"
sleep 1

echo "Escribiendo en archivo test.txt con echo..."
echo "hola!" > test.txt
check_operation "Escribir archivo test.txt con echo"
sleep 1

echo "Leyendo el archivo test.txt"
cat test.txt
check_operation "Lectura del archivo test.txt"
sleep 1

echo "Estadisticas..."
stat test.txt
check_operation "Estadisticas del archivo test.txt"

echo "Eliminando el archivo test.txt"
rm test.txt
check_operation "Eliminar archivo test.txt con rm"
sleep 1

echo "Creando un directiorio dentro de prueba"
mkdir "testDir"
check_operation "Creacion de un directorio testDir con mkdir"
sleep 1

echo "Listar archivos del directorio"
ls
check_operation "Listado de archivos en el directorio"
sleep 1

echo "Accediendo al nuevo directorio..."
cd "testDir"
check_operation "Acceso al nuevo directorio testDir"
cd ..
sleep 1

echo "Eliminando directorio creado..."
rmdir "testDir"
check_operation "Eliminacion del directorio testDir"


cd ..


echo "Creando estructura de subdirectorios anidados..."
mkdir -p dir1/dir2/dir3/dir4
check_operation "Crear estructura dir1/dir2/dir3/dir4"
sleep 1

echo "Accediendo a dir1/dir2/dir3/dir4"
cd dir1/dir2/dir3/dir4
check_operation "Acceso a dir1/dir2/dir3/dir4"
sleep 1

echo "Creando archivo en el directorio más profundo"
echo "contenido de prueba" > archivo_en_dir4.txt
check_operation "Crear archivo en dir4"
sleep 1

echo "Leyendo archivo creado en dir4"
cat archivo_en_dir4.txt
check_operation "Lectura del archivo en dir4"
sleep 1

cd ../../../../

echo "Eliminando estructura de subdirectorios..."
rm -rf dir1
check_operation "Eliminar estructura dir1/dir2/dir3/dir4"
sleep 1


echo "Desmontando el filesystem FUSE..."
umount /fisopfs/prueba
check_operation "Desmontar el filesystem FUSE"


echo "Pruebas completadas."
