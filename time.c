#include <stdio.h>
#include <sys/time.h> // for gettimeofday()
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

// main function to find the execution time of a C program
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Inserte algun argumento o no inserte tantos\n");
        exit(1);
    }
    struct timeval inicio, fin;

    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        gettimeofday(&inicio, NULL);  
        execvp( argv[1], &argv[1]);
    }
    else if (pid > 0)
    {
             
        wait(NULL);
        gettimeofday(&fin, NULL);

        double secs = (double)(fin.tv_usec - inicio.tv_usec) / 1000000 + (double)(fin.tv_sec - inicio.tv_sec);
        printf("Tiempo de ejecucion: %f segundos\n", secs);


    }
     return 0;
}