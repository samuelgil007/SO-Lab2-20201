#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
char directorio[30];
int salidaGlobal, numPath, numPath = 2;
char **paths;
char error_message[30] = "An error has occurred\n";

void type_prompt()
{
    static int first_time = 1;
    if (first_time)
    { // limpiar pantalla si se usa por primera vez
        const char *CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
        first_time = 0;
    }

    if (strcmp(directorio, "") == 0)
    {
        printf("wish>"); //MOSTRAR SHELL
    }
    else
    {
        printf("wishl>%s>", directorio); //MOSTRAR SHELL
    }
}
int estaEnElPath(char command[])
{
    for (int i = 0; i < numPath; i++)
    {
        if (paths[i] != NULL)
        {

            char esBin[30];
            strcpy(esBin, paths[i]);
            strcat(esBin, command);
            if (access(esBin, X_OK) == 0)
            {
                return i;
            }

            // Si no se encuentra directorio no se ejecuta el comando
            if (i == (numPath - 1))
            {
                /* printf("No se puede encontrar ese comando en el path: %s\n", command); */
                return 99;
            }
        }
    }
    return 99;
}

void leer_comando(char cmd[], char *par[], char line[])
{
    int i = 0;
    char *array[100];
    strtok(line, "\n");

    // Extract the first token
    char *token = strtok(line, " \r");
    // loop through the string to extract all other tokens

    while (token != NULL)
    {

        array[i++] = token;
        token = strtok(NULL, " ");
    }

    // La primera palabra es el comando
    strcpy(cmd, array[0]);
    if (strcmp(cmd, "path") == 0)
    {
        numPath = i;
    }
    // Los otros son los parametros

    for (int j = 0; j < i; j++)
    {
        par[j] = array[j];
    }
    par[i] = NULL; //Terminar la lista de parametros
}

void ejecutar_comando(char command[], char *parameters[], char line[])
{
    //identifica comandos, si no lo encuentra hace un bin por defecto
    char cmd[100];

    if (strcmp(command, "exit") == 0)
    {
        if (parameters[1] != NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            salidaGlobal = 1;
            return;
        }
    }
    else if (strcmp(command, "path") == 0)
    {

        if (numPath == 0)
        {
            char **newPaths = malloc((1) * sizeof(char *));
            newPaths[0] = "l";
            paths = newPaths;
        }
        else
        {
            char **newPaths = malloc((numPath) * sizeof(char *));
            for (int i = 0; i < numPath; i++)
            {
                newPaths[i] = parameters[i];
            }
            paths = newPaths;
        }
        return;
    }
    else if (strcmp(command, "cd") == 0)
    {
        if (chdir(parameters[1]) != 0 && parameters[1] != NULL)
        {
            /* printf("No se puede encontrar ese directorio %s\n", error_message); */
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else if (parameters[1] == NULL)
        {
            /* printf("Error 0 o mas de 1 argumento \n"); */
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            /* printf("ingresó a %s \n",parameters[1]); //menos .. */
            //chdir(parameters[1]); vuela
            strcat(directorio, parameters[1]);
        }
        return;
    }
    int esBin = estaEnElPath(command);
    if (esBin != 99)
    {
        if (fork() != 0)
        {
            wait(NULL);
        }
        else
        {
            strcpy(cmd, paths[esBin]);
            strcat(cmd, command);
            execv(cmd, parameters); // ejecutar comando
        }
    }
}

int main(int argc, char *argv[])
{
    char command[100], *parameters[20];
    numPath = 2;
    paths = malloc((numPath) * sizeof(char *));
    paths[1] = "/bin/";

    //Modo batch
    if (argc == 2)
    {
        FILE *fp = stdin;
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            /* printf("Error con el archivo de texto \n"); */
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
            //return 0;
        }
        else
        {
            int linex = 0;
            salidaGlobal = 0;
            while (salidaGlobal != 1)
            {
                char *line = NULL;
                size_t size = 0;
                linex = getline(&line, &size, fp);
                if (linex == -1)
                {
                    if (argc != 1 && feof(fp))
                    {
                        fclose(fp);
                        exit(0);
                    }
                }

                if (strcmp(line, "") == 0)
                {
                    salidaGlobal = 1;
                }
                else
                {
                    /*  printf("%s \n", line); */
                    leer_comando(command, parameters, line);
                    ejecutar_comando(command, parameters, line);
                }
            }
        }
        fclose(fp);
    }
    //Modo interactivo
    else if (argc == 1)
    {
        salidaGlobal = 0;
        while (salidaGlobal != 1)
        {
            type_prompt(); //mostrar pantalla prompt
            char *line = NULL;
            size_t size = 0;
            getline(&line, &size, stdin);
            leer_comando(command, parameters, line); // leer el input
            ejecutar_comando(command, parameters, line);
        }
    }
    else
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
   exit(0);
}
