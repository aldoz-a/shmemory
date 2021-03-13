// mmap.h - header mini-libreria IPC con memory mapped file
#include <pthread.h>
#include <stdbool.h>

// struttura per i dati condivisi
typedef struct {
    pthread_mutex_t mutex;      // mutex comune ai processi
    pthread_cond_t  cond;       // condition variable comune ai processi
    bool            data_ready; // flag per dati disponibili (true=ready)
    size_t          len;        // lunghezza campo data
    char            data[1];    // dati da condividere
} shdata;

// prototipi globali
shdata *memMapOpen(const char *mmname, size_t len, bool create);
void   memMapClose(const char *mmname, shdata *ptr);
void   memMapRead(shdata *ptr, void *buf, size_t count);
void   memMapWrite(shdata *ptr, const void *buf, size_t count);
