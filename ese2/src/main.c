/* Estendere Ese_1 affinché il processo Consumatore riceva da riga di comando una lista di pathname di
 * file di testo. Per ogni file di testo, il processo Consumatore crea un processo Produttore, il quale invia il
 * contenuto del file di testo al Consumatore attraverso un’unica PIPE.
 * La PIPE e’ un bytes stream channel. Se due o piu’ Produttori scrivono sulla stessa PIPE, come possiamo
 * distinguere i dati scritti da un Produttore, dai dati scritti da un altro Produttore?
 * Soluzione: Il produttore antempone il numero di byte che intende scrivere, prima dei dati effettivi.
 *              --------------------------------------------------------------------------------
 * PIPE read end ← | 2 | byte-1 byte-2| 4| byte-1 byte-2 byte-3 byte-4| ----
 *              --------------------------------------------------------------------------------
 * Il contenuto della PIPE sopra riportata mostra due chunk di dati.
 * Il primo chunk ha dimensione 2 bytes, mentre il secondo ha dimensione 4 bytes/
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CHUNK_SIZE 100

struct Item {
    ssize_t size;
    char value[CHUNK_SIZE];
};

int main(int argc, char *argv[]) {

    /* Command line arguments checking */
    if (argc == 1) {
        printf("ERROR: missing argument(s)\nUSAGE: %s <file1.txt> <file2.txt> ...", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Pipe creation */
    int pd[2];
    if (pipe(pd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    /* For each file start a new process for reading the file */
    printf("<Consumer> making %d subprocesses...\n", (argc - 1));
    for (int i = 0; i < (argc - 1); i++) {
        char * filename = argv[i+1];
        pid_t pid = fork();
        if (pid == -1) {
            printf("Fork failed: %s will not be read!\n", filename);
        } else if (pid == 0) {

            if (close(pd[0]) == -1) {
                perror("close pipe[0] failed");
                exit(EXIT_FAILURE);
            }

            printf("<Producer> text file: %s\n", filename);
            int fd = open(filename, O_RDONLY);
            if (fd == -1) {
                perror("open file failed");
                exit(EXIT_FAILURE);
            }

            struct Item item;
            do {
                /* Read a chunk from file and copy it in item.value
                 * then set the size equal to the bytes read */
                item.size = read(fd, item.value, sizeof(item.value));
                if (item.size == -1) {
                    perror("Read from file failed");
                    exit(EXIT_FAILURE);
                }

                if (item.size > 0) {
                    /* Write the read bytes into the pipe */
                    ssize_t byte2send = sizeof(item.size) + item.size;
                    ssize_t byWritten = write(pd[1], &item, byte2send);
                    if (byWritten == -1) {
                        perror("write on pipe failed");
                        exit(EXIT_FAILURE);
                    }
                }
            } while (item.size > 0);

            if (close(fd) == -1 || close(pd[1] == -1)) {
                perror("close failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }

    /* Consumer code */
    /* Close write-end of the pipe */
    if (close(pd[1]) == -1) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }

    char buffer[CHUNK_SIZE+1];
    ssize_t byRead = -1;

    do {
        ssize_t size;
        if ((byRead = read(pd[0], &size, sizeof(ssize_t)))== -1) {
            printf("<Consumer> pipe is broken\n");
        } else if (byRead == 0) {
            printf("<Consumer> all pipe's write-ends are closed\n");
        } else if (byRead != sizeof(ssize_t)) {
            printf("<Consumer> nothing to read\n");
        } else {
            byRead = read(pd[0], buffer, size);
            if (byRead == -1) {
                printf("<Consumer> pipe is broken\n");
            } else if (byRead == 0) {
                printf("<Consumer> all pipe's write-ends are closed\n");
            } else if (byRead != size) {
                printf("<Consumer> nothing to read\n");
            } else {
                buffer[size] = '\0';
                printf("<Consumer> line: %s\n", buffer);
            }
        }
    } while (byRead > 0);

    if (close(pd[0]) == -1) {
        perror("Closing pipe failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
