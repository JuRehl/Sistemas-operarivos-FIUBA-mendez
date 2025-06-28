# Análisis de strace (parte 1)

## Introducción

`strace` (abreviación de *system-call trace*) es una herramienta de línea de comandos que permite observar las llamadas al sistema (syscalls) que realiza un proceso mientras se ejecuta. Esto es útil para depurar errores, entender cómo interactúa un programa con el kernel o simplemente para aprender cómo funciona internamente una aplicación.

En esta primera parte del trabajo se explorará el funcionamiento de `strace` mediante su uso sobre diferentes comandos sencillos. Luego se analizará su comportamiento cuando se ejecuta sobre sí misma.

Nota: Muchas llamadas al sistema adicionales a las aquí listadas pueden aparecer, como `open` para abrir librerías compartidas o archivos de configuración. Estas llamadas provienen del cargador dinámico (ld-linux.so) y de la biblioteca estándar (libc), que el proceso usa para funcionar. En esta salida limitada nos centramos en las syscalls que están más directamente relacionadas con la funcionalidad principal de cada comando.


---

## Comandos analizados

Se analizaron tres comandos simples de Unix con `strace`, limitando el rastreo a las llamadas al sistema más representativas: `read`, `write`, `open` y `execve`.

### 1. `ls`

```bash
strace -e trace=read,write,open,execve ls
```
Salida

```bash
strace -e trace=read,write,open,execve ls
execve("/usr/bin/ls", ["ls"], 0x7ffd109278b0 /* 65 vars */) = 0
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\220\243\2\0\0\0\0\0"..., 832) = 832
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
read(3, "nodev\tsysfs\nnodev\ttmpfs\nnodev\tbd"..., 1024) = 421
read(3, "", 1024)                       = 0
write(1, "ps  ps.c  strace.md  timeout  ti"..., 40ps  ps.c  strace.md  timeout  timeout.c
) = 40
+++ exited with 0 +++
```
Syscalls observadas:

**execve**: ejecuta el binario ls.

**read**: lee archivos de configuración y contenido del directorio.

**write**: imprime los nombres de archivos en pantalla.

### 2. `cat /etc/passwd`

Este comando muestra el contenido del archivo `/etc/passwd`.

```bash
strace -e trace=read,write,open,execve cat /etc/passwd
```
Salida

```bash
strace -e trace=read,write,open,execve cat /etc/passwd
execve("/usr/bin/cat", ["cat", "/etc/passwd"], 0x7fffea201d68 /* 65 vars */) = 0
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\220\243\2\0\0\0\0\0"..., 832) = 832
read(3, "root:x:0:0:root:/root:/bin/bash\n"..., 131072) = 3194
write(1, "root:x:0:0:root:/root:/bin/bash\n"..., 3194root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
bin:x:2:2:bin:/bin:/usr/sbin/nologin
sys:x:3:3:sys:/dev:/usr/sbin/nologin
sync:x:4:65534:sync:/bin:/bin/sync
games:x:5:60:games:/usr/games:/usr/sbin/nologin
man:x:6:12:man:/var/cache/man:/usr/sbin/nologin
lp:x:7:7:lp:/var/spool/lpd:/usr/sbin/nologin
mail:x:8:8:mail:/var/mail:/usr/sbin/nologin
news:x:9:9:news:/var/spool/news:/usr/sbin/nologin
uucp:x:10:10:uucp:/var/spool/uucp:/usr/sbin/nologin
proxy:x:13:13:proxy:/bin:/usr/sbin/nologin
www-data:x:33:33:www-data:/var/www:/usr/sbin/nologin
backup:x:34:34:backup:/var/backups:/usr/sbin/nologin
list:x:38:38:Mailing List Manager:/var/list:/usr/sbin/nologin
irc:x:39:39:ircd:/run/ircd:/usr/sbin/nologin
gnats:x:41:41:Gnats Bug-Reporting System (admin):/var/lib/gnats:/usr/sbin/nologin
nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin
systemd-network:x:100:102:systemd Network Management,,,:/run/systemd:/usr/sbin/nologin
systemd-resolve:x:101:103:systemd Resolver,,,:/run/systemd:/usr/sbin/nologin
messagebus:x:102:105::/nonexistent:/usr/sbin/nologin
systemd-timesync:x:103:106:systemd Time Synchronization,,,:/run/systemd:/usr/sbin/nologin
syslog:x:104:111::/home/syslog:/usr/sbin/nologin
_apt:x:105:65534::/nonexistent:/usr/sbin/nologin
tss:x:106:113:TPM software stack,,,:/var/lib/tpm:/bin/false
uuidd:x:107:116::/run/uuidd:/usr/sbin/nologin
systemd-oom:x:108:117:systemd Userspace OOM Killer,,,:/run/systemd:/usr/sbin/nologin
tcpdump:x:109:118::/nonexistent:/usr/sbin/nologin
usbmux:x:111:46:usbmux daemon,,,:/var/lib/usbmux:/usr/sbin/nologin
dnsmasq:x:112:65534:dnsmasq,,,:/var/lib/misc:/usr/sbin/nologin
kernoops:x:113:65534:Kernel Oops Tracking Daemon,,,:/:/usr/sbin/nologin
avahi:x:114:121:Avahi mDNS daemon,,,:/run/avahi-daemon:/usr/sbin/nologin
cups-pk-helper:x:115:122:user for cups-pk-helper service,,,:/home/cups-pk-helper:/usr/sbin/nologin
rtkit:x:116:123:RealtimeKit,,,:/proc:/usr/sbin/nologin
whoopsie:x:117:124::/nonexistent:/bin/false
sssd:x:118:125:SSSD system user,,,:/var/lib/sss:/usr/sbin/nologin
speech-dispatcher:x:119:29:Speech Dispatcher,,,:/run/speech-dispatcher:/bin/false
fwupd-refresh:x:120:126:fwupd-refresh user,,,:/run/systemd:/usr/sbin/nologin
nm-openvpn:x:121:127:NetworkManager OpenVPN,,,:/var/lib/openvpn/chroot:/usr/sbin/nologin
saned:x:122:129::/var/lib/saned:/usr/sbin/nologin
colord:x:123:130:colord colour management daemon,,,:/var/lib/colord:/usr/sbin/nologin
geoclue:x:124:131::/var/lib/geoclue:/usr/sbin/nologin
pulse:x:125:132:PulseAudio daemon,,,:/run/pulse:/usr/sbin/nologin
gnome-initial-setup:x:126:65534::/run/gnome-initial-setup/:/bin/false
hplip:x:127:7:HPLIP system user,,,:/run/hplip:/bin/false
gdm:x:128:134:Gnome Display Manager:/var/lib/gdm3:/bin/false
ju:x:1000:1000:Ju,,,:/home/ju:/bin/bash
snapd-range-524288-root:x:524288:524288::/nonexistent:/usr/bin/false
snap_daemon:x:584788:584788::/nonexistent:/usr/bin/false
dhcpcd:x:129:65534:DHCP Client Daemon,,,:/usr/lib/dhcpcd:/bin/false
cups-browsed:x:130:122::/nonexistent:/usr/sbin/nologin
polkitd:x:998:998:User for polkitd:/:/usr/sbin/nologin
gnome-remote-desktop:x:995:995:GNOME Remote Desktop:/var/lib/gnome-remote-desktop:/usr/sbin/nologin
) = 3194
read(3, "", 131072)                     = 0
+++ exited with 0 +++
```
Syscalls observadas

**execve**: Invoca el binario /usr/bin/cat con el argumento /etc/passwd.

**read**:
Primero lee encabezados binarios (ELF).
Luego lee el contenido del archivo /etc/passwd.

**write**: Imprime todo el contenido del archivo en la salida estándar (pantalla). 

##
Tanto para el caso de cat como para ls, se observan múltiples llamadas `read` que corresponden a la lectura tanto del archivo ejecutable (cabecera ELF) como a la lectura de archivos de configuración o contenido que el comando manipula. Por ejemplo, en cat /etc/passwd se lee el contenido completo del archivo /etc/passwd para mostrarlo en pantalla.
##

### 3. `date`

Este comando imprime la fecha y hora actual del sistema.

#### Comando ejecutado

```bash
strace -e trace=read,write,open,execve date
```
Salida
```bash
strace -e trace=read,write,open,execve date
execve("/usr/bin/date", ["date"], 0x7fff1e93f3a0 /* 65 vars */) = 0
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\220\243\2\0\0\0\0\0"..., 832) = 832
read(3, "TZif2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 4096) = 1076
read(3, "TZif2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 4096) = 666
write(1, "vie 20 jun 2025 17:17:09 -03\n", 29vie 20 jun 2025 17:17:09 -03
) = 29
+++ exited with 0 +++
```
Syscalls observadas

**execve**: Ejecuta el binario /usr/bin/date.

**read**: Lee contenido de archivos, probablemente relacionados a configuración de zona horaria como /etc/localtime.

**write**: Escribe la fecha y hora actual en la salida estándar.

##
En general, la llamada execve invoca el ejecutable y arranca el proceso. Previamente, el sistema operativo debe leer el archivo binario ELF (vía llamadas read) para cargar el programa en memoria.
##
## Explicación general del output de `strace`

La utilidad `strace` permite observar en tiempo real todas las llamadas al sistema (syscalls) que realiza un programa durante su ejecución. La salida que presenta está compuesta por líneas donde se muestra, para cada syscall:

- El nombre de la llamada al sistema (por ejemplo, `read`, `write`, `execve`).
- Los argumentos con los que se invoca dicha llamada.
- El valor de retorno de la syscall, que puede representar la cantidad de bytes procesados, un descriptor de archivo, o un código de error.

Por ejemplo, la línea:

```bash
write(1, "hola\n", 5) = 5
```
indica que el proceso intentó escribir 5 bytes (el texto "hola\n") en el descriptor de archivo número 1 (que corresponde a la salida estándar, stdout), y que efectivamente se escribieron 5 bytes.

## ¿Aparecen más llamadas de las esperadas?

En los comandos analizados, se observó que la cantidad de llamadas al sistema reportadas por strace es mayor a las que uno podría esperar al analizar sólo la funcionalidad básica de cada comando. Esto se debe a que las syscalls reflejan todas las interacciones del proceso con el sistema operativo, no sólo las relacionadas con la tarea visible.

Por ejemplo:

En el comando ls se observan llamadas para leer archivos de configuración relacionados con el entorno del sistema, como variables locales o configuraciones de terminal.

En el comando date se accede a archivos binarios relacionados con la configuración de la zona horaria, como el archivo /etc/localtime.

En el comando cat se ve que primero se leen encabezados binarios ELF, correspondientes a la carga del ejecutable, además de la lectura del archivo /etc/passwd.

**Estas llamadas adicionales provienen principalmente de:**

*El proceso de carga del binario:* La llamada execve inicia la ejecución del programa y el kernel lee el encabezado ELF del ejecutable para cargarlo en memoria.

*Bibliotecas dinámicas:* La carga automática de librerías compartidas (como libc), que también requieren lectura y configuración.

*Archivos de configuración del entorno:* Como configuraciones locales, zonas horarias, o información de la terminal.

*Inicialización del entorno de ejecución:* El runtime del lenguaje y otros procesos internos preparan el entorno para la ejecución del programa.

Estas observaciones demuestran que incluso los comandos más sencillos realizan múltiples interacciones con el sistema operativo para funcionar correctamente.

# Análisis de `strace` ejecutándose sobre sí mismo

## Comando usado

Para analizar las syscalls `ptrace` y `wait` que utiliza `strace` al ejecutarse sobre sí mismo, se usó el siguiente comando:

```bash
strace -f -e trace=ptrace,waitpid,wait4,execve strace ls
```
Salida
```bash
 strace -f -e trace=ptrace,waitpid,wait4,execve strace ls
execve("/usr/bin/strace", ["strace", "ls"], 0x7ffc79d954d0 /* 65 vars */) = 0
strace: Process 8882 attached
[pid  8880] ptrace(PTRACE_SEIZE, 8882, NULL, 0) = -1 EPERM (Operación no permitida)
[pid  8880] wait4(8882,  <unfinished ...>
[pid  8882] +++ killed by SIGKILL +++
<... wait4 resumed>[{WIFSIGNALED(s) && WTERMSIG(s) == SIGKILL}], 0, NULL) = 8882
--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_KILLED, si_pid=8882, si_uid=1000, si_status=SIGKILL, si_utime=0, si_stime=0} ---
strace: Process 8886 attached
[pid  8880] wait4(8886,  <unfinished ...>
[pid  8886] ptrace(PTRACE_TRACEME)      = -1 EPERM (Operación no permitida)
strace: test_ptrace_get_syscall_info: PTRACE_TRACEME: Operación no permitida
[pid  8886] +++ exited with 1 +++
<... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 1}], 0, NULL) = 8886
--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED, si_pid=8886, si_uid=1000, si_status=1, si_utime=0, si_stime=0} ---
strace: Process 8887 attached
[pid  8887] ptrace(PTRACE_TRACEME)      = -1 EPERM (Operación no permitida)
[pid  8880] wait4(-1, strace: ptrace(PTRACE_TRACEME, ...): Operación no permitida
 <unfinished ...>
[pid  8887] +++ exited with 1 +++
<... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 1}], __WALL, NULL) = 8887
--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED, si_pid=8887, si_uid=1000, si_status=1, si_utime=0, si_stime=0} ---
wait4(-1, 0x7ffd4e3f20fc, WNOHANG|__WALL, NULL) = -1 ECHILD (No hay ningún proceso hijo)
ptrace(PTRACE_SETOPTIONS, 8887, NULL, PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACEEXEC|PTRACE_O_TRACEEXIT) = -1 ESRCH (No existe el proceso)
+++ exited with 1 +++
wait4(-1, 0x7ffd4e3f20fc, __WALL, NULL) = -1 ECHILD (No hay ningún proceso hijo)
+++ exited with 1 +++
```
## Análisis de las syscalls observadas

**execve:** Se ejecuta el binario /usr/bin/strace con el argumento ls. Esto inicia el proceso strace que a su vez va a rastrear el programa ls.

**ptrace:**
PTRACE_SEIZE y PTRACE_TRACEME son llamadas usadas para iniciar el trazado de un proceso hijo.
En este caso, varias llamadas ptrace devuelven error EPERM (Operación no permitida). Esto puede deberse a restricciones de permisos en el sistema (por ejemplo, políticas de seguridad como Yama en Linux que restringen ptrace a procesos no privilegiados).

strace usa ptrace para controlar la ejecución de procesos hijos y poder interceptar sus syscalls.

**wait4 (una variante de wait):**
Se usa para esperar cambios de estado en procesos hijos (como terminaciones, señales, stops).
Se observa que wait4 retorna información sobre señales recibidas o salida de procesos hijos.
También se muestra que algunos llamados a wait4 devuelven error ECHILD cuando no hay procesos hijos para esperar.

**Señales:**
Se observan señales como SIGKILL y SIGCHLD que indican terminación o muerte de procesos hijos.
Estas señales son manejadas por strace para coordinar el seguimiento del proceso rastreado.

### Conclusión de la parte 1
No se observan llamadas a fork o clone, lo que indica que estos procesos no crean nuevos procesos o threads, manteniéndose simples para este análisis. Esto facilita el rastreo y comprensión de las llamadas al sistema. En procesos más complejos, strace debe lidiar con múltiples procesos o threads, complicando el seguimiento.


# Implementación strace (parte 2)

Este programa es una implementación simplificada del comando `strace`, que permite interceptar e imprimir las llamadas al sistema realizadas por un proceso.

## Explicación comentada del código

```c
if (argc < 2) {
    fprintf(stderr, "Uso: %s <programa> [args...]\n", argv[0]);
    exit(1);
}
```
Verificamos que se haya pasado un programa para ejecutar. Si no, mostramos un mensaje de error.

```c
pid_t child = fork();
```
Creamos un nuevo proceso hijo. El hijo va a ejecutar el programa que queremos tracear, el padre lo va a monitorear.

### Proceso hijo
```c
if (child == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execvp(argv[1], &argv[1]);
    perror("execvp");
    exit(1);
}
```
El hijo avisa al kernel que quiere ser traceado (PTRACE_TRACEME), y luego ejecuta el programa objetivo con execvp. Si execvp falla, imprime el error y termina.

### Proceso padre 
```c
else {
    int status;
    struct user_regs_struct regs;
    int in_syscall = 0;
```
Inicializamos variables:
 status para ver el estado del hijo con waitpid, regs contiene los registros del proceso hijo, in_syscall nos permite distinguir entre entrada/salida de syscall.
```c
    waitpid(child, &status, 0);
```
Esperamos a que el hijo haga su execvp y se detenga antes de empezar.

### Bucle principal
```c
    while (1) {
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        waitpid(child, &status, 0);

        if (WIFEXITED(status)) break;

        ptrace(PTRACE_GETREGS, child, NULL, &regs);
```
PTRACE_SYSCALL permite avanzar al siguiente punto de entrada o salida de syscall. Esperamos que el hijo se detenga.
Si terminó (WIFEXITED), salimos.
Si no, leemos los registros del hijo.

```c
        if (!in_syscall) {
            printf("syscall(%lld) = ", regs.orig_rax);
            in_syscall = 1;
        } else {
            printf("%lld\n", regs.rax);
            in_syscall = 0;
        }
    }
```
Cuando se entra a una syscall, se imprime el número (orig_rax). Al salir, se imprime el valor de retorno (rax).
 Alternamos con la variable in_syscall.

##  Evidencia de funcionamiento

Se realizaron pruebas con programas reales (`ls`, `echo`) y se compararon los resultados con `strace`.

### strace sobre `/bin/ls`
```bash
syscall brk (12) = 99049012142080
syscall mmap (9) = 134186484682752
syscall access (21) = 18446744073709551614
syscall openat (257) = 3
syscall fstat (5) = 0
syscall mmap (9) = 134186484600832
syscall close (3) = 0
syscall openat (257) = 3
syscall read (0) = 832
syscall fstat (5) = 0
syscall mmap (9) = 134186484416512
syscall mmap (9) = 134186484441088
syscall mmap (9) = 134186484559872
syscall mmap (9) = 134186484584448
syscall mmap (9) = 134186484592640
syscall close (3) = 0
syscall openat (257) = 3
syscall read (0) = 832
syscall pread64 (17) = 784
syscall fstat (5) = 0
syscall pread64 (17) = 784
syscall mmap (9) = 134186480435200
syscall mmap (9) = 134186480599040
syscall mmap (9) = 134186482204672
syscall mmap (9) = 134186482528256
syscall mmap (9) = 134186482552832
syscall close (3) = 0
syscall openat (257) = 3
syscall read (0) = 832
syscall fstat (5) = 0
syscall mmap (9) = 134186483785728
syscall mmap (9) = 134186483793920
syscall mmap (9) = 134186484244480
syscall mmap (9) = 134186484408320
syscall close (3) = 0
syscall mmap (9) = 134186483773440
syscall arch_prctl (158) = 0
syscall set_tid_address (218) = 12145
syscall set_robust_list (273) = 0
syscall rseq (334) = 0
syscall mprotect (10) = 0
syscall mprotect (10) = 0
syscall mprotect (10) = 0
syscall mprotect (10) = 0
syscall mprotect (10) = 0
syscall prlimit64 (302) = 0
syscall munmap (11) = 0
syscall statfs (137) = 18446744073709551614
syscall statfs (137) = 18446744073709551614
syscall getrandom (318) = 8
syscall brk (12) = 99049012142080
syscall brk (12) = 99049012277248
syscall openat (257) = 3
syscall fstat (5) = 0
syscall read (0) = 421
syscall read (0) = 0
syscall close (3) = 0
syscall access (21) = 18446744073709551614
syscall openat (257) = 3
syscall fstat (5) = 0
syscall mmap (9) = 134186469949440
syscall close (3) = 0
syscall ioctl (16) = 0
syscall ioctl (16) = 0
syscall openat (257) = 3
syscall fstat (5) = 0
syscall getdents64 (217) = 184
syscall getdents64 (217) = 0
syscall close (3) = 0
syscall fstat (5) = 0
mini_strace  strace.c  strace.md  syscalls_table.c
syscall write (1) = 51
syscall close (3) = 0
syscall close (3) = 0
```
### Comparación con strace
```bash
strace /bin/ls
execve("/bin/ls", ["/bin/ls"], 0x7ffd2478d910 /* 66 vars */) = 0
brk(NULL)                               = 0x62ec56c3d000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x72826cd7e000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No existe el archivo o el directorio)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=78407, ...}) = 0
mmap(NULL, 78407, PROT_READ, MAP_PRIVATE, 3, 0) = 0x72826cd6a000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libselinux.so.1", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=174472, ...}) = 0
mmap(NULL, 181960, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x72826cd3d000
mmap(0x72826cd43000, 118784, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x6000) = 0x72826cd43000
mmap(0x72826cd60000, 24576, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x23000) = 0x72826cd60000
mmap(0x72826cd66000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x29000) = 0x72826cd66000
mmap(0x72826cd68000, 5832, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x72826cd68000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\220\243\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
fstat(3, {st_mode=S_IFREG|0755, st_size=2125328, ...}) = 0
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
mmap(NULL, 2170256, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x72826ca00000
mmap(0x72826ca28000, 1605632, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x28000) = 0x72826ca28000
mmap(0x72826cbb0000, 323584, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b0000) = 0x72826cbb0000
mmap(0x72826cbff000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1fe000) = 0x72826cbff000
mmap(0x72826cc05000, 52624, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x72826cc05000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libpcre2-8.so.0", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=625344, ...}) = 0
mmap(NULL, 627472, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x72826cca3000
mmap(0x72826cca5000, 450560, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2000) = 0x72826cca5000
mmap(0x72826cd13000, 163840, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x70000) = 0x72826cd13000
mmap(0x72826cd3b000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x97000) = 0x72826cd3b000
close(3)                                = 0
mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x72826cca0000
arch_prctl(ARCH_SET_FS, 0x72826cca0800) = 0
set_tid_address(0x72826cca0ad0)         = 12131
set_robust_list(0x72826cca0ae0, 24)     = 0
rseq(0x72826cca1120, 0x20, 0, 0x53053053) = 0
mprotect(0x72826cbff000, 16384, PROT_READ) = 0
mprotect(0x72826cd3b000, 4096, PROT_READ) = 0
mprotect(0x72826cd66000, 4096, PROT_READ) = 0
mprotect(0x62ec237cd000, 8192, PROT_READ) = 0
mprotect(0x72826cdb6000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0x72826cd6a000, 78407)           = 0
statfs("/sys/fs/selinux", 0x7ffe0bbd4610) = -1 ENOENT (No existe el archivo o el directorio)
statfs("/selinux", 0x7ffe0bbd4610)      = -1 ENOENT (No existe el archivo o el directorio)
getrandom("\x1c\xc2\x04\x75\x52\x58\x5e\x05", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0x62ec56c3d000
brk(0x62ec56c5e000)                     = 0x62ec56c5e000
openat(AT_FDCWD, "/proc/filesystems", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0444, st_size=0, ...}) = 0
read(3, "nodev\tsysfs\nnodev\ttmpfs\nnodev\tbd"..., 1024) = 421
read(3, "", 1024)                       = 0
close(3)                                = 0
access("/etc/selinux/config", F_OK)     = -1 ENOENT (No existe el archivo o el directorio)
openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=8398816, ...}) = 0
mmap(NULL, 8398816, PROT_READ, MAP_PRIVATE, 3, 0) = 0x72826c000000
close(3)                                = 0
ioctl(1, TCGETS, {c_iflag=BRKINT|ICRNL|IXON|IXANY|IMAXBEL|IUTF8, c_oflag=NL0|CR0|TAB0|BS0|VT0|FF0|OPOST|ONLCR, c_cflag=B38400|CS8|CREAD|HUPCL, c_lflag=ISIG|ICANON|ECHO|ECHOE|ECHOK|IEXTEN|ECHOCTL|ECHOKE, ...}) = 0
ioctl(1, TIOCGWINSZ, {ws_row=8, ws_col=152, ws_xpixel=0, ws_ypixel=0}) = 0
openat(AT_FDCWD, ".", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 3
fstat(3, {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
getdents64(3, 0x62ec56c43ce0 /* 6 entries */, 32768) = 184
getdents64(3, 0x62ec56c43ce0 /* 0 entries */, 32768) = 0
close(3)                                = 0
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0), ...}) = 0
write(1, "mini_strace  strace.c  strace.md"..., 51mini_strace  strace.c  strace.md  syscalls_table.c
) = 51
close(1)                                = 0
close(2)                                = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```
Aunque la salida del strace implementado es más simple, se puede verificar que los números de syscall y sus valores de retorno coinciden con las llamadas detectadas por strace. Esto confirma que la implementación logra interceptar correctamente las llamadas al sistema y reportarlas con su resultado, cumpliendo con los requisitos pedidos.

### Adicional
Para generar el archivo con los nombres de las syscalls se debe ejecutar
```bash
grep '^#define __NR_' /usr/include/x86_64-linux-gnu/asm/unistd_64.h \
  | awk '{printf("[%s] = \"%s\",\n", $3, substr($2, 6))}' > syscalls_table.c

```
