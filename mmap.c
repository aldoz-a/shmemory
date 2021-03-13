// mmap.c - implementazione mini-libreria IPC con memory mapped file
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mmap.h"

// memMapOpen() - apre un memory mapped file
shdata *memMapOpen(
    const char *mmname,
    size_t     len,
    bool       create)
{
    shdata *ptr;

    // test se modo create o modo open di un mmfile già creato
    if (create) {
        // apre un memory mapped file (il file "mmname" è creato in /dev/shm)
        int fd;
        if ((fd = shm_open(mmname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
            return NULL;    // esce con errore

        // tronca un memory mapped file
        if (ftruncate(fd, sizeof(shdata) + len) == -1)
            return NULL;    // esce con errore

        // mappa un memory mapped file
        if ((ptr = mmap(NULL, sizeof(shdata) + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
            return NULL;    // esce con errore

        // chiude la shared memory: questo non compromette il map eseguito
        close(fd);

        // init mutex in modo "shared memory"
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&ptr->mutex, &attr);
        pthread_mutexattr_destroy(&attr);

        // init condition variable in modo "shared memory"
        pthread_condattr_t attrcond;
        pthread_condattr_init(&attrcond);
        pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&ptr->cond, &attrcond);
        pthread_condattr_destroy(&attrcond);

        // init altri dati
        ptr->data_ready = false;
        ptr->len = len;
    }
    else {
        // apre un memory mapped file (il file "shmname" è creato in /dev/shm)
        int fd;
        if ((fd = shm_open(mmname, O_RDWR, S_IRUSR | S_IWUSR)) == -1)
            return NULL;    // esce con errore

        // mappa un memory mapped file
        if ((ptr = mmap(NULL, sizeof(shdata) + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
            return NULL;    // esce con errore

        // chiude la shared memory: questo non compromette il map eseguito
        close(fd);
    }

    // ritorna il pointer
    return ptr;
}

// memMapClose() - chiude un memory mapped file
void memMapClose(
    const char *mmname,
    shdata     *ptr)
{
    // rilascio tutte le risorse acquisite
    pthread_mutex_destroy(&ptr->mutex);
    pthread_cond_destroy(&ptr->cond);
    munmap(ptr, sizeof(shdata) + ptr->len);
    shm_unlink(mmname);
}

// memMapRead() - legge dati dal mapped-file
void memMapRead(
    shdata *ptr,
    void   *buf,
    size_t count)
{
    // lock mutex
    pthread_mutex_lock(&ptr->mutex);

    // aspetta la condizione
    while (!ptr->data_ready)
        pthread_cond_wait(&ptr->cond, &ptr->mutex);

    // legge i dati dal mapped-file e segnala la condizione
    memcpy(buf, ptr->data, count);
    ptr->data_ready = false;
    pthread_cond_signal(&ptr->cond);

    // unlock mutex
    pthread_mutex_unlock(&ptr->mutex);
}

// memMapWrite() - scrive dati nel mapped-file
void memMapWrite(
    shdata     *ptr,
    const void *buf,
    size_t     count)
{
    // lock mutex
    pthread_mutex_lock(&ptr->mutex);

    // aspetta la condizione
    while (ptr->data_ready)
        pthread_cond_wait(&ptr->cond, &ptr->mutex);

    // scrive i dati sul mapped-file esegnala la condizione
    memcpy(ptr->data, buf, count);
    ptr->data_ready = true;
    pthread_cond_signal(&ptr->cond);

    // unlock mutex
    pthread_mutex_unlock(&ptr->mutex);
}
