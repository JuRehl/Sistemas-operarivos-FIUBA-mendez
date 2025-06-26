# shell

### Búsqueda en $PATH

- Las diferencias radican en que execve es la syscall en si, esta reemplaza al proceso actual por uno nuevo. Tiene una cierta cantidad de argumentos obligatorios y no busca el binario en el PATH, por el contrario, se le debe pasar la ruta completa. En cambio, la familia de weappers de exec que encontramos en la libería de C, están basados justamente en execve. Es decir, la familia de wrappers llama eventualmente a execve pero estos tienen distintas funcionalidades. Por ejemplo, la cantidad de parámetros que recibe cada función varia según su utilidad. Como execvp, que utiliza un PATH y no se le debe pasar la ruta completa como seria en el caso de execve.

- Exec puede fallar, esto puede ser por distintas razones, como que el comando no exista, no hay permiso para ejecutarlo, el archivo no es válido, entre otras. Cuando falla no reemplaza el proceso por el contrario, sigue el flujo del programa normalmente. Para este caso, la shell imprime un mensaje de error, especificando que es un error del exec, y luego sale del programa con un exit.

---

### Procesos en segundo plano

- El uso de señales, particularmente para este caso `SIGCHILD`, es necesario porque permite que la shell se entere inmediatamente que un proceso hijo finalizó. Esto lo hace sin la necesidad de quedarse esperando, lo que es fundamental para los procesos en segundo plano como estuvimos trabajando en la parte 5. La shell no debe esperar a que los procesos terminen y puede seguir ejecutando comandos, independientemente de estos procesos en segundo plano o categorizados de *background*.

- Para manejar estos procesos en segundo plano se utilizó un *handler* asociado a la señal `SIGCHILD`, que es mandada por el sistema operativo cuando un proceso hijo termina. Este *handler* se configuró gracias a la syscall `sigaction(2)`. Dentro del *handler* se llama a `waitpid` con el flag `WNOHAND`, ya que nos recolecta de manera no bloqueante los procesos hijos que ya terminaron. Además, se imprime por pantalla al usuario el `pid` del proceso finalizado como lo indica la consigna.

- Luego, en `run_cmd()`, se evalúan los procesos en segundo plano, que imprimen su información y el *handler* se encarga de liberarlos al finalizar. Si es un proceso de primer plano, simplemente se hace el `waitpid` bloqueante y muestra su estado de finalización.

---

### Flujo estándar

- El operador `2>&1` en Bash redirige el flujo de error estándar (`stderr`, descriptor de archivo 2) al mismo destino que el flujo de salida estándar (`stdout`, descriptor de archivo 1). Esto significa que cualquier mensaje de error generado por un comando se enviará al mismo lugar que la salida normal.

  Salidas bash:
    ```
    root@PC-Facu:~$ ls -C /home /noexiste >out.txt 2>&1
    root@PC-Facu:~$ cat out.txt
    ls: cannot access '/noexiste': No such file or directory
    /home:
    ```
    ```
    root@PC-Facu:~$ ls -C /home /noexiste 2>&1 >out.txt
    ls: cannot access '/noexiste': No such file or directory
    root@PC-Facu:~$ cat out.txt
    /home:
    ```
    - El orden de las redirecciones afecta el resultado:
      - `out.txt 2>&1`: Ambos (`stdout` y `stderr`) se redirigen a `out.txt`.
      - `2>&1 >out.txt`: `stderr` se redirige a la consola, mientras que `stdout` se redirige a `out.txt`.


  Salidas shell propia:
    ```
    $ ls -C /home /noexiste >out.txt 2>&1
            Program: [ls -C /home /noexiste >out.txt 2>&1] exited, status: 0 
    $ cat out.txt
    ls: cannot access '/noexiste': No such file or directory
    /home:
            Program: [cat out.txt] exited, status: 0 
    ```
    ```
    $ ls -C /home /noexiste 2>&1 >out.txt
            Program: [ls -C /home /noexiste 2>&1 >out.txt] exited, status: 0 
    $ cat out.txt
    ls: cannot access '/noexiste': No such file or directory
    /home:
            Program: [cat out.txt] exited, status: 0
    ```
    - En nuestra shell, el orden de las redirecciones no afecta el resultado: 
      - `out.txt 2>&1`: Ambos (`stdout` y `stderr`) se redirigen a `out.txt`.
      - `2>&1 >out.txt`: Ambos (`stdout` y `stderr`) se redirigen a `out.txt`.
    
    Esta diferencia se debe a que en nuestra implementacion, se verifica primero cualquier redireccion del `stdout` y luego las redirecciones de `stderr` ya sea la normal o la combinada.

---

### Tuberías múltiples

- Si cambia, ya que al tener múltiples pipes, solo se muestra el exit code el último comando ejecutado. Ya que la salida del primer comando se redirecciona a la entrada del segundo y así sucesivamente. Cuando estemos en el último pipe la shell mostrará el exit code del último de la fila. 

- Si alguno de los comandos falla, solo se registra la salida del último pipe. Es decir, si alguno de los pipes intermedios falla, el error ocurre pero el código de salida no refleja el fallo ya que no es el último comando.  
    
    Salidas bash:
    ```
    root@PC-Facu:~$ echo "hello" | grep "hello" | wc -l
    1
    root@PC-Facu:~$ echo $?
    0 // No falla ningun comando
    ```
    ```
    root@PC-Facu:~$ echo "hello" | grep "world" | wc -l
    0
    root@PC-Facu:~$ echo $?
    0 // Falla el grep pero no cambia el exit code porque wc -l no falla
    ```
    ```
    root@PC-Facu:~$ echo "hello" | grep "hello" | ls /noexiste
    ls: cannot access '/noexiste': No such file or directory
    root@PC-Facu:~$ echo $?
    2 // Error del ls
    ```
    ```
    root@PC-Facu:~$ echo "hello" | grep "aaao" | ls /noexiste
    ls: cannot access '/noexiste': No such file or directory
    root@PC-Facu:~$ echo $?
    2 // Error del ls aunque falle el grep
    ```
    Salidas shell propia:
    ```
     (/root)
    $ echo "hello" | grep "hello" | wc -l
    1
     (/root) 
    $ echo $?
    0
    ```
    ```
     (/root) 
    $ echo "hello" | grep "world" | wc -l
    0
     (/root)
    $ echo $?
    0
    ```
    ```
     (/root)
    $ echo "hello" | grep "hello" | ls /noexiste
    ls: cannot access '/noexiste': No such file or directory
     (/root) 
    $ echo $?
    0
    ```
    ```
     (/root)
    $ echo "hello" | grep "aaao" | ls /noexiste
    ls: cannot access '/noexiste': No such file or directory
     (/root) 
    $ echo $?
    0 
    ```
    En la shell propia, el exit code no refleja correctamente el del último comando en un pipeline. En lugar de eso, incluso cuando el último comando falla, reporta 0.
---

### Variables de entorno temporarias

- Se deben setear las variables de entorno temporarias luego del fork, ya que si lo hacemos previamente seteariamos las variables para el proceso inicial de la shell , es decir el proceso padre, ya que `setenv()` modifica el entorno del proceso actual. Todas las ejecuciones posteriores tendrían las mismas variables lo que rompe el concepto de "variable de entorno temporarias". En cambio, si las seteamos luego de hacer el fork, solo el hijo modificaría sus variables de entorno, por lo que serían realmente variables temporales. 

- El comportamiento no sería el mismo, ya que al usar la familia de wrappers de _exec(3)_, solo se usan las variables del entorno agregadas para el momento de ejecutar el exec, Este entorno es completamente **personalizado y aislado**. Por el contrario, al usar `setenv()` las variables quedan grabadas para el proceso completo, en este caso el proceso hijo de la shell, y cualquier proceso hijo que pueda surgir de este. 

- En vez de hacer `setenv()` por cada variable de entorno temporal se puede: 
    Crear un arreglo `char *envp[]` en el cuál clonamos todas las entradas de environ al nuevo `envp[]`
    Cambiar todas las variables deseadas o agregar nuevas
    Terminar el arreglo con `NULL`
    Pasar `envp` como el tercer argumento de `execve()` o cualquiera de la familia de wrappers.
    Luego cuando llamemos a una funcion de la familia _exec(3)_ le pasamos como parámetro el arreglo de claves designado previamente.
  Así nos aseguramos que el proceso ejecutado tenga únicamente las variables de entorno especificadas, sin alterar el entorno del proceso actual
---

### Pseudo-variables

- Otras variables mágicas estándar útiles en la shell son:
    - `$$`: Muestra el PID del proceso de la shell actual. Permite identificar el proceso en ejecución.
        Ejemplo de uso en bash:
        ```
        juana-pc:~$ echo $$ > pid.txt
        juana-pc:~$ cat pid.txt
            26891
        ```

    - `$!`: Devuelve el PID del último proceso ejecutado en segundo plano. Sirve para hacer seguimiento a un proceso background.
        Ejemplo de uso:
        ```
        juana-pc:~$ sleep 60 &
            [1] 38923
        juana-pc:~$ echo "El PID del sleep es: $!"
            El PID del sleep es: 38923
        ```

    - `$_`: Retorna el último argumento del comando anterior. Es útil para reutilizar el último argumento ingresado y no escribirlo de nuevo.
        Ejemplo de uso:
        ```
        juana-pc:~$ mkdir test && cd $_
        juana-pc:~test$ pwd
            /home/juana-pc/test
        ```
    
    - `$0`: Representa el nombre del script o comando actual. Si se ejecuta desde la terminal, muestra el nombre de la shell. Si se ejecuta desde un script, muestra el nombre del script
        Ejemplo de uso:
        ```
        juana-pc:~$ echo "El nombre del script es: $0"
            El nombre del script es: bash
        ```

---

### Comandos built-in

- `pwd`: Sí, `pwd` podría implementarse sin necesidad de ser un comando built-in. Esto se debe a que `pwd` simplemente imprime el directorio de trabajo actual, y esto puede lograrse ejecutando un programa externo como /bin/pwd o utilizando la syscall getcwd() desde un programa independiente.

- `cd`: No, `cd` no puede implementarse correctamente sin ser un comando built-in. Esto se debe a que cambiar el directorio de trabajo actual afecta al proceso que ejecuta el comando. Si `cd` se implementara como un programa externo, solo cambiaría el directorio de trabajo del proceso hijo que ejecuta el programa, pero no afectaría al proceso de la shell principal.
  
- Comandos como `true` y `false` son built-ins en muchas shells, aunque podrían implementarse como programas externos. El motivo de hacerlos built-ins es la eficiencia: estos comandos son extremadamente simples (solo devuelven un código de salida) y no necesitan crear un proceso hijo para ejecutarse. De manera similar, `pwd` se implementa como built-in en muchas shells para mejorar la eficiencia y la flexibilidad.

---

### Historial

---
