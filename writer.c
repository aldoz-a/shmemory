// writer.c - main processo figlio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "mmap.h"
#include "data.h"

// funzione main()
int main(int argc, char* argv[])
{
    // apro il memory mapped file
    printf("processo %d partito\n", getpid());
    shdata *data;
    if ((data = memMapOpen(MMAP_NAME, sizeof(Data), false)) == NULL) {
        // errore di apertura
        printf("%s: non posso aprire il memory mapped file (%s)\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // loop di scrittura messaggi per il reader
    Data my_data;
    my_data.index = 0;
    for (;;) {
        // test index per forzare l'uscita
        if (my_data.index == N_MESSAGES) {
            // il processo esce per indice raggiunto
            printf("processo %d terminato (text=%s index=%ld)\n", getpid(), my_data.text, my_data.index);
            exit(EXIT_SUCCESS);
        }

        // compongo il messaggio e lo invio
        my_data.index++;
        snprintf(my_data.text, sizeof(my_data.text), "un-messaggio-di-test:%ld", my_data.index);
        memMapWrite(data, &my_data, sizeof(Data));
    }

    // il processo esce per altro motivo (errore: non gestito in questa versione)
    printf("processo %d terminato con errore (%s)\n", getpid(), strerror(errno));
    exit(EXIT_FAILURE);
}
