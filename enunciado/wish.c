#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

char directorio[30];
int salidaGlobal, nproces = 1, numPath = 2;
char **paths;
char error_message[30] = "An error has occurred\n";
int redirection, tamanoItems, Proces[100];
int settingLine(char *line, ssize_t linex);
int forks[100], contFork = 0;

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

            char esta[30];
            strcpy(esta, paths[i]);
            strcat(esta, "/");
            strcat(esta, command);

            if (access(esta, X_OK) == 0)
            {
                return i;
            }

            // Si no se encuentra directorio no se ejecuta el comando
            if (i == (numPath - 1) || strcmp(command, "\n") == 0 || command == NULL)
            {
                /* printf("No se puede encontrar ese comando en el path: %s\n", command); */
                return 99;
            }
        }
    }
    return 99;
}

int leer_comando(char cmd[], char *par[], char line[])
{
    memset(Proces, 0, sizeof(Proces));
    int i = 0;
    nproces = 1;
    int positionRedi = -1;
    char *array[100];
    strtok(line, "\t\a\n\r");

    // Extract the first token
    char *token = strtok(line, " \t\a\n\r");
    // loop through the string to extract all other tokens
    while (token != NULL)
    {
        //controlo que si hay un > se sepa.
        if (strcmp(token, ">") == 0)
        {
            if (redirection == 0 && i != 0)
            {
                redirection = 1;
                positionRedi = i;
            }
            else
            {
                redirection = -1;
                //positionRedi= -1;
            }
        }
        if (strcmp(token, "&") == 0)
        {
            Proces[nproces] = i;
            nproces++;
        }
        array[i++] = token;
        token = strtok(NULL, " \t\a\n\r");
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

    //pregunto por el caso favorable (para ver luego si si es favorable en verdad)
    if (redirection == 1 && positionRedi != -1)
    {

        if ((i - (positionRedi + 1)) != 1)
        {
            //quiere decir que hay 2 o mas parametros ala derecha de > , se deja la
            //posicion del primer archivo para guardar alli el mensaje de error

            redirection = -1;
        }
        if (array[positionRedi + 1] == NULL || positionRedi == (i - 1))
        {
            //como cumplio que desps del > hay nada se pone como malo
            redirection = -1;
            positionRedi = -1;

            //exit(1);
        }
    }

    return positionRedi;
}

void ejecutar_comando(char command[], char *parameters[], char line[], int posRedi)
{
    char cmd[100];
    //identifica comandos, si no lo encuentra hace un bin por defecto

    if (command != NULL)
    {
        int esBin = estaEnElPath(command);
        if (strcmp(command, "exit") == 0)
        {
            if (parameters[1] != NULL)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else
            {
                salidaGlobal = 1;
                exit(0);
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
                /* printf("llega aqui \n");
            printf("No se puede encontrar ese directorio %s\n", parameters[1]); */
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else if (parameters[1] == NULL)
            {
                /*  printf("Error 0 o mas de 1 argumento \n"); */
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
        else if (esBin != 99)
        {
            pid_t child = fork();
            forks[contFork] = child;
            forks[contFork + 1] = 777;
            contFork++;
            if (child == 0)
            {
                if (redirection == 0)
                {
                    strcpy(cmd, paths[esBin]);
                    strcat(cmd, "/");
                    strcat(cmd, command);
                    execv(cmd, parameters); // ejecutar comando
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
                else
                {
                    char *finalParams[posRedi];
                    char *documento = parameters[posRedi + 1];
                    //logica crear el archivo y poner a escribir en el
                    char outputFile[32];
                    sscanf(documento, "%s", outputFile);
                    int fd = creat(outputFile, 0644); // create new file
                    int newfd = dup2(fd, 1);          // redirects stdout to outputFile
                    if (newfd == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }
                    //pregunto si hay un comando bueno
                    if (redirection == 1)
                    {
                        for (int i = 0; i < posRedi; i++)
                        {
                            finalParams[i] = parameters[i];
                            //printf("%s \n", finalParams[i]);
                        }
                        finalParams[posRedi] = NULL;
                        //printf("el documento se llama: %s \n", documento);
                    }
                    strcpy(cmd, paths[esBin]);
                    strcat(cmd, "/");
                    strcat(cmd, command);
                    //ejecuto el archivo o error
                    if (redirection == 1 && posRedi != -1)
                    {
                        int pepe = execv(cmd, finalParams);
                        if (pepe == -1)
                        {
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }
                        /* printf("rip\n"); */
                        exit(1);
                    }
                    else if (redirection == -1)
                    {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(0);
                    }
                }
            }
        }
        else
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

int main(int argc, char *argv[])
{
    char command[100], *parameters[20];
    numPath = 2;
    paths = malloc((numPath) * sizeof(char *));
    paths[1] = "/bin";
    contFork = 0;
    forks[0] = 777;

    //////////////////////////////////////////////Modo batch
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
            ssize_t linex = 0;
            salidaGlobal = 0;
            redirection = 0;
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
                tamanoItems = settingLine(line, linex);
                if (tamanoItems == -1)
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                    ;
                }
                if (tamanoItems == 0)
                {
                    continue;
                }
                char *comandos[100];
                int k = 0;
                // Extract the first token
                char *token = strtok(line, "&");
                // loop through the string to extract all other tokens
                while (token != NULL)
                {
                    comandos[k++] = token;
                    token = strtok(NULL, "&");
                }
                /*  printf("%d \n", k);  */
                if (k != 0)
                {
                    //paralelos
                    for (int i = 0; i < k; i++)
                    {
                        int posRedi = leer_comando(command, parameters, comandos[i]); // leer el input
                        ejecutar_comando(command, parameters, comandos[i], posRedi);
                        redirection = 0;
                    }
                    int status;
                    for (int f = 0; forks[f] != 777; f++)
                    {
                        waitpid(forks[f], &status, 0);
                    }
                }
            }
            fclose(fp);
        }
    }
    ////////////////////////////////////////Modo interactivo
    else if (argc == 1)
    {
        salidaGlobal = 0;
        redirection = 0;
        ssize_t linex = 0;
        while (salidaGlobal == 0)
        {
            type_prompt(); //mostrar pantalla prompt
            char *line = NULL;
            size_t size = 0;
            linex = getline(&line, &size, stdin);
            if (linex <= 1)
            {
                continue;
            }
            tamanoItems = settingLine(line, linex);
            if (tamanoItems == -1)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            if (tamanoItems == 0)
            {
                continue;
            }
            char *comandos[100];
            int k = 0;
            // Extract the first token
            char *token = strtok(line, "&");
            // loop through the string to extract all other tokens
            while (token != NULL)
            {
                comandos[k++] = token;
                token = strtok(NULL, "&");
            }
            /*  printf("%d \n", k);  */
            if (k != 0)
            {
                //paralelos
                for (int i = 0; i < k; i++)
                {
                    /* printf("%s \n", comandos[i]); */
                    int posRedi = leer_comando(command, parameters, comandos[i]); // leer el input
                    ejecutar_comando(command, parameters, comandos[i], posRedi);
                    redirection = 0;
                }
                int status;
                for (int f = 0; forks[f] != 777; f++)
                {
                    waitpid(forks[f], &status, 0);
                }
            }
        }
    }
    else
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    exit(0);
}

//se le da formato a la linea de entrada (convirtiendo todo lo raro a espacio)
//para poder separar por espacios de manera sencilla
int settingLine(char *line, ssize_t linex)
{
    //numero de parametros buenos (no espacios)
    int wordsNum = 1;
    //se asigna al ultimo espacio del comando el nulo
    line[linex - 1] = '\0';

    // aqui se convierte todo lo raro a espacios
    for (int i = 0; i < linex; i++)
    {
        if (line[i] == '\t' || line[i] == '\n' || line[i] == '\a' || line[i] == '\r')
            line[i] = ' ';
    }

    // borrar espacios del principio
    while (*line == ' ')
    {
        line++;
        linex--;
    }
    //si no hay nada de entrada se devuelve
    if (linex <= 1)
        return 0;

    // se cuenta el numero de palabras (buenas)
    for (int i = 1; line[i] != '\0'; i++)
    {
        if (line[i] != ' ' && line[i - 1] == ' ')
            wordsNum++;
    }
    return wordsNum;
}
