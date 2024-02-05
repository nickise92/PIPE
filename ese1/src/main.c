/* Esercizio 1:
 * Scrivere un programma che implementi il paradigma Produttore-Consumatore basato su PIPE.
 * Il processo Consumatore deve ricevere da riga di comando il pathname di uni file di testo. 
* Successivamente, il processo Consumatore crea una PIPE, e un processo Produttore. Il 
 * processo Produttore legge il contenuto del file in chunk di massimo 100 caratteri. Ogni 
 * chunk letto viene inviato al processo Consumatore attraverso la PIPE. Il processo Consumatore
 * stampa il contenuto del file ricevuto per mezzo della PIPE.
 **/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BF_SIZE 100

int main(int argc, char * argv[]) {

    /* Check argomenti da riga di comando */
    if (argc < 2) {
        printf("Pathname del file di testo mancante o non valido!\n");
        exit(0);
    }
    char * path = argv[1];


    /* Creazione della pipe */
    int pd[2];
    
    if (pipe(pd) == -1) {
        perror("pipe");
        exit(1);
    } 
    
    /* Creazione del processo Produttore */
    int processPid = fork();
    if (processPid == -1) {

        /* Errore */
        perror("fork");
        exit(1);

    } else if (processPid == 0) { /* Figlio - Produttore */

        /* Chiudo il read-end della pipe nel processo Produttore */
        if (close(pd[0]) == -1) {
            perror("close");
            exit(1);
        }

        /* Apro il file in sola lettura */
        int fd;
        if ((fd = open(path, O_RDONLY)) == -1) {
            perror("open");
            exit(1);
        }

        /* Creo un buffer e leggo il primo chunk */
        char buffer[BF_SIZE];
        size_t byRead;
        while ((byRead = read(fd, buffer, BF_SIZE)) > 0) {

            /* Scrivo sulla pipe il contenuto del buffer */
            if (write(pd[1], buffer, byRead) == -1) {
                perror("write");
                exit(1);
            }
        }

        if (byRead == -1) {
            perror("read");
            exit(1);
        }

        /* Close file descriptor and pipe descriptor */
        if ( close(fd) == -1 || close(pd[1]) == -1) {
            perror("close failed");
            exit(EXIT_FAILURE);
        }
    } else if (processPid > 0) { /* Padre - Consumatore */

        /* Chiudo il descrittore di scrittura della pipe */
        if (close(pd[1]) == -1) {
            perror("close");
            exit(1);
        }

        /* Creo il buffer di lettura & leggo dalla pipe fino ad esaurimento dati */
        char buffer[BF_SIZE];
        size_t byRead;
        do {
            if ((byRead = read(pd[0], buffer, BF_SIZE)) == -1) {
                perror("read");
                exit(1);
            }

            /* Stampo su STDOUT il contenuto del buffer */
            if (write(STDOUT_FILENO, buffer, byRead) == -1) {
                perror("write");
                exit(1);
            }

        } while (byRead > 0);

        close(pd[1]);
    }



    printf("\n");
    return 0;
}
