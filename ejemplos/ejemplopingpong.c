#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
/*pipe 1 se va a utilizar para comunicar del padre al hijo y pipe 2 se va a utilizar para comunicar del hijo al padre*/


int main(){
    int pipe1[2],pipe2[2];
    if (pipe(pipe1)==-1 || pipe(pipe2)==-1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    printf("Hola soy PID %d: \n", getpid());
    printf("- IDs del primer pipe: [%d,%d]\n", pipe1[0],pipe2[1]);
    printf("- IDs del primer pipe: [%d,%d]\n",pipe2[0],pipe2[1]);

    pid_t pid=fork();
    if (pid<0){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid>0){ //proceso del padre
        close(pipe1[0]);//el de lectura del padre
        close(pipe2[1]);//el de escritura del hijo
        srandom(time(NULL));
        int valor=random();
        printf("\nDonde fork me devuelve %d \n",pid);
        printf(" - getpid me devuelve: %d\n",getpid());
        printf(" - getppid me devuelve: %d\n",getppid());
        printf(" - valor random: %d\n",valor);
        printf(" - envío valor %d a través de fd=%d\n",valor,pipe1[1]);
        write(pipe1[1],&valor,sizeof(int));
        close(pipe1[1]);

        int recibido;
        read(pipe2[0],&recibido,sizeof(int));
        printf("\nHola, de nuevo PID %d: \n",getpid());
        printf("- recibí valor %d vía fd=%d\n",recibido,pipe2[0]);
        close(pipe2[0]);
        wait(NULL);
    }else{
        close(pipe1[1]);
        close(pipe2[0]);
        int recibido;
        read(pipe1[0],&recibido,sizeof(int));
        printf("\n Donde fork me devuelve 0: \n ");
        printf("- getpid me devuelve: %d\n",getpid());
        printf("- getppid me devuelve: %d\n",getppid());
        printf("- recibo valor %d vía fd=%d\n", recibido,pipe2[0]);
        printf("- reenvío valor en fd=%d y termino\n",pipe2[1]);
        write(pipe2[1],&recibido,sizeof(int));

        close(pipe1[0]);
        close(pipe2[1]);
    }
    return 0;
}