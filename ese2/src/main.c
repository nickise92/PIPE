/* Esercizio 2:
 * Estendere l'esercizio 1 affinché il processo Consumatore riceva da riga di comando
 * una lista di pathname di file di testo. Per ogni file di testo, il processo Consumatore
 * crea un processo Produttore, il quale invia il contenuto del file di testo al Consumatore
 * attraverso un'unica PIPE.
 * La PIPE è un bytes stream channel. Se due o più Produttori scrivono sulla stessa PIPE,
 * come possiamo distinguere i dati scritti da un Produttore, dai dati scritti da un altro Produttore?
 * Soluzione:
 * Il Produttore antepone il numero di byte che intende scrivere, prima dei dati effettivi.
 * PIPE read end <- | 2 | byte'1 byte'2 | 4 | byte-1 byte-2 byte-3 byte-4 | ...
 *
 * Il contenuto della PIPE sopra riportata mostra due chunk di dati.
 * Il primo chunk ha dimensione 2 bytes, mentre il secondo ha dimensione 4 bytes.
 *
 * Consiglio: definire la seguente struttura:
 *
 * struct Item {
 *      ssize_t size;               // contiene il numero di byte inviati
 *      char value[MSG_BYTE];       // contiene i dati effettivi inviati
 * };
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../inc/consumer.h"
#include "../inc/producer.h"


int main (int argc, char * argv[]) {

    /* Check arguments */
    if (argc == 1) {
        printf("ERROR! Not enough arguments.\nUsage: %s <file1.txt> <file2.txt> ...\n", argv[0]);
        exit(0);
    }
}

