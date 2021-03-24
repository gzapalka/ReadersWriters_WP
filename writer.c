#include "header.h"

#define W1 0
#define W2 1
#define W3 2
#define SP 3
#define SC 4
#define M 6

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
    sleep(rand() % 3);
}

void writing()
{
    char mess = pid % 10 + 64;
    sleeping();
    strcpy(sharedMemory, &mess);
    printf("%d Writer | Write: %c\n", pid, mess);
    fflush(stdout);
}

void task()
{

    while (1)
    {
        //sleeping();
        waitS(W2);
        signalS(M);
        if (semctl(semID, M, GETVAL, NULL) == 1)
        {
            waitS(SC);
        }
        signalS(W2);

        waitS(SP);
        writing();
        signalS(SP);

        waitS(W2);
        waitS(M);
        if (semctl(semID, M, GETVAL) == 0)
        {
            signalS(SC);
        }
        signalS(W2);
    }
}

void sigHandler(){
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandler);
    srand(time(NULL));
    librarySize = convertToLong(argv[1]);
    init();
    sharedMemory = shmat(shmID, NULL, 0);
    task();

    return 0;
}