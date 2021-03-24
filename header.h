#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

/*#define W 0
#define R 1
#define M 2*/

struct sembuf w[7] = {{0, -1, 0}, {1, -1, 0}, {2, -1, 0}, {3, -1, 0}, {4, -1, 0}, {5, -1, 0}, {6, -1, 0}};
struct sembuf s[7] = {{0, +1, 0}, {1, +1, 0}, {2, +1, 0}, {3, +1, 0}, {4, +1, 0}, {5, +1, 0}, {6, +1, 0}};

key_t key;
int semID;
int shmID;
void *sharedMemory;
int pid;
long memSize = sizeof(char);
//void *shmem;

void waitS(int semNumber)
{
    if (semop(semID, w + semNumber, 1) == -1)
    {
        perror("Semop waits");
    }
}

void signalS(int semNumber)
{
    if (semop(semID, s + semNumber, 1) == -1)
    {
        perror("Semop signals");
    }
}

long convertToLong(char *arg)
{
    char *buff;
    errno = 0;

    long value = strtol(arg, &buff, 10);
    ;
    if (errno == ERANGE && (value == LONG_MAX || value == LONG_MIN) || (errno != 0 && value == 0))
    {
        perror("Blad strtol");
        exit(EXIT_FAILURE);
    }

    if (buff == arg)
    {
        printf("Argument nie jest liczba");
        exit(EXIT_FAILURE);
    }

    return value;
}

key_t createKey()
{
    key_t key = ftok(".", 'G');
    if (key == -1)
    {
        perror("Blad tworzenia klucza");
        exit(EXIT_FAILURE);
    }
    return key;
}

void getSemID()
{
    semID = semget(key, 7, IPC_CREAT | IPC_EXCL | 0600); //tu moga byc problemy z main, usunelam IPC_EXCL
    if ((semID == -1) && (errno == EEXIST))
    {
        //printf("Dolaczam do zbioru sem\n");
        semID = semget(key, 7, 0600);
    }
    else
    {
        if (semID == -1)
        {
            perror("Blad semget");
            exit(EXIT_FAILURE);
        }
    }
}

void getShmID()
{
    shmID = shmget(key, memSize, IPC_CREAT | 0600);
    if (shmID == -1)
    {
        perror("Blad shmget");
        exit(EXIT_FAILURE);
    }
    sharedMemory = shmat(shmID, NULL, 0);
    if (sharedMemory == (void *)-1)
    {
        perror("Blad shmat");
        exit(EXIT_FAILURE);
    }
}

long getSystemLimits()
{
    FILE *temp;
    char limit_buf[32];
    temp = popen("bash -c 'ulimit -u'", "r");
    if (!temp)
    {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    if (fgets(limit_buf, 32, temp) == NULL && pclose(temp) == -1)
    {
        perror("Nie udalo sie odczytac limitu procesow");
        exit(EXIT_FAILURE);
    }
    return convertToLong(limit_buf);
}

long getCurrentProcess()
{
    FILE *temp;
    char limit_buf[32];
    temp = popen("ps -u | wc -l", "r");
    if (!temp)
    {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    if (fgets(limit_buf, 32, temp) == NULL && pclose(temp) == -1)
    {
        perror("Nie udalo sie odczytac liczby dzialajacych procesow");
        exit(EXIT_FAILURE);
    }

    return convertToLong(limit_buf);
}

void checkProcessLimits(long readersCount, long writersCount)
{
    if (getSystemLimits() - getCurrentProcess() - 1 < writersCount + readersCount)
    {
        printf("Zbyt duza liczba procesow");
        exit(EXIT_FAILURE);
    }
}