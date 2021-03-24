#include "header.h"
#include <wait.h>

#define M 0


long writersCount;
long readersCount;
long librarySize;

void checkArg(int argc, char **argv)
{
    if (argc = !3)
    {
        printf("Bledna liczba argumentow");
        exit(EXIT_FAILURE);
    }
    writersCount = convertToLong(argv[1]);
    readersCount = convertToLong(argv[2]);
    librarySize = convertToLong(argv[3]);

    if (!(writersCount && readersCount && memSize))
    {
        printf("Argument nie moze byc zerem");
        exit(EXIT_FAILURE);
    }
}

void initSem()
{

    for(int i=0; i<5; i++){
        if(semctl(semID, i, SETVAL,1)==-1){
            perror("Semctl setval1");
            exit(EXIT_FAILURE);
        }
    }
    if(semctl(semID, 5, SETVAL,0)==-1){
    perror("Semctl setval2");
        exit(EXIT_FAILURE);
    }

    if(semctl(semID, 6, SETVAL,0)==-1){
        perror("Semctl setval3");
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < 7; i++)
    {
        printf("SEM[%d] = %d\n", i, semctl(semID, i, GETVAL, NULL));
        fflush(stdout);
    }
    printf("SemID: %d\n", semID);
    fflush(stdout);
}

void init()
{
    key = createKey();
    printf("SemKey: %d\n", key);
    fflush(stdout);
    getSemID();
    initSem();
    getShmID();
}

void createProcess(char* arg)
{
    for (int i = 0; i < writersCount; i++)
    {
        switch (fork())
        {
        case -1:
            perror("Blad fork");
            exit(EXIT_FAILURE);
        case 0:
            execl("./writer", "writer",arg, NULL);
            perror("Blad execl");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < readersCount; i++)
    {
        switch (fork())
        {
        case -1:
            perror("Blad fork");
            exit(EXIT_FAILURE);
        case 0:
            execl("./reader", "reader",arg, NULL);
            perror("Blad execl");
            exit(EXIT_FAILURE);
        }
    }
}

void sigHandler()
{
    if (semctl(semID, 3, IPC_RMID, NULL) == -1)
    {
        perror("Blad zwolnienia semaforow\n");
    }
    if (shmctl(shmID, IPC_RMID, NULL) == -1)
    {
        perror("Blad zwolnienia pamieci dzielonej\n");
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandler);
    checkArg(argc, argv);
    checkProcessLimits(readersCount, writersCount);
    init();
    createProcess(argv[2]);
    for (int i = 0; i < 3; i++)
    {
        if (waitpid(0, NULL, 0) == -1)
        {
            perror("Blad wait");
        }
    }

    //clearUp();
    exit(EXIT_SUCCESS);
}