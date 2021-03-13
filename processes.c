// processes.c - main processo padre
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "mmap.h"
#include "data.h"

// funzione main()
int main(int argc, char* argv[])
{
    // creo il memory mapped file
    shdata *data;
    if ((data = memMapOpen(MMAP_NAME, sizeof(Data), true)) == NULL) {
        // errore di creazione
        printf("%s: non posso creare il memory mapped file (%s)\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // crea i processi figli
    pid_t pid1, pid2;
    (pid1 = fork()) && (pid2 = fork());

    // test pid processi
    if (pid1 == 0) {
        // sono il figlio 1
        printf("sono il figlio 1 (%d): eseguo il nuovo processo\n", getpid());
        char *pathname = "reader";
        char *newargv[] = { pathname, NULL };
        execv(pathname, newargv);
        exit(EXIT_FAILURE);   // exec non ritorna mai
    }
    else if (pid2 == 0) {
        // sono il figlio 2
        printf("sono il figlio 2 (%d): eseguo il nuovo processo\n", getpid());
        char *pathname = "writer";
        char *newargv[] = { pathname, NULL };
        execv(pathname, newargv);
        exit(EXIT_FAILURE);   // exec non ritorna mai
    }
    else if (pid1 > 0 && pid2 > 0) {
        // sono il padre
        printf("sono il padre (%d): attendo la terminazione dei figli\n", getpid());
        int status;
        pid_t wpid;
        while ((wpid = wait(&status)) > 0)
            printf("sono il padre (%d): figlio %d terminato (%d)\n", getpid(), (int)wpid, status);

        // rimuovo il memory mapped file ed esco
        printf("%s: processi terminati\n", argv[0]);
        memMapClose(MMAP_NAME, data);
        exit(EXIT_SUCCESS);
    }
    else {
        // errore nella fork(): rimuovo il memory mapped file ed esco
        printf("%s: fork error (%s)\n", argv[0], strerror(errno));
        memMapClose(MMAP_NAME, data);
        exit(EXIT_FAILURE);
    }
}
