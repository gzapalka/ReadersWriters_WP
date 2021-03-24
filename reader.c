#include "header.h"
#define W1 0
#define W2 1
#define W3 2
#define SP 3
#define SC 4
#define M 5

long librarySize;

void init()
{
    key = createKey();
    getSemID();
    getShmID();
    pid = getpid();
}

void sleeping()
{
   usleep(rand() % 3);
}

void reading()
{
    //sleeping();
    char *mess = (char *)sharedMemory;
    printf("%d Reader | Read: %c\n", pid, *mess);
    fflush(stdout);
}

void task()
{
    while (1)
    {

        sleeping();
        waitS(W3);
        waitS(SC);
        waitS(W1);

        while (librarySize < semctl(semID, M, GETVAL, NULL))
        {
            usleep(10);
        }

        signalS(M);
        if (semctl(semID, M, GETVAL) == 1)
        {
            waitS(SP);
        }
        signalS(W1);
        signalS(SC);
        signalS(W3);

        reading();

        waitS(W1);
        waitS(M);

        if (semctl(semID, M, GETVAL, NULL) == 0)
        {
            signalS(SP);
        }

        signalS(W1);
    }
}

void sigHandler()
{
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandler);
    librarySize = convertToLong(argv[1]);
    srand(time(NULL));
    init();
    sharedMemory = shmat(shmID, NULL, 0);
    task();

    return 0;
}