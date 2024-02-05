

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MSG_BYTE 100

typedef struct Item {
    ssize_t size;
    char value[MSG_BYTE];
}Item;

int main (int argc, char * argv[]) {

    /* Check arguments */ 
    if (argc < 2) {
        printf("ERROR! Too few arguments!\n");
        printf("USAGE: ./ese2 <pathToFile1> <pathToFile2> ...\n");
        exit(0);
    }

    char * pathlist[argc-1];
    for (int i = 0; i < argc-1; i++) {
        pathlist[i] = argv[i+1];
    }

    /* Create a PIPE in parent */
    int pipeDescriptor[2];

    if (pipe(pipeDescriptor) == -1) {
        perror("pipe");
        exit(1);
    }

    int processList[argc-1]; 
    for (int i = 0; i < argc-1; i++) {
        if ((processList[i] = fork()) == -1) {
            perror("fork");
            exit(1);
        }
    }
    
    for (int i = 0; i < argc-1; i++) {
        if (processList[i] == 0) { /* Produttore */ 
            
            /* Close the PIPE read-end */ 
            if (close(pipeDescriptor[0]) == -1) {
                    perror("close");
                    exit(1);
            }

            /* Open file in read only mode */
            int fd;
            if ((fd = open(pathlist[i], O_RDONLY)) == -1) {
                perror("open");
                exit(1);
            }

            /* Read the file and save it in Item struct */
            Item item;
            size_t byteRead;
            char buffer[MSG_BYTE];

            while ((byteRead = read(fd, buffer, MSG_BYTE)) > 0) {

                if (byteRead == -1) {
                    perror("read");
                    exit(1);
                }

                strcpy(item.value, buffer);
                item.size = byteRead;
                
                /* Write on PIPE the message content size */
                if (write(pipeDescriptor[1], &item, item.size + sizeof(item.size)) == -1) {
                    perror("write");
                    exit(1);
                }
            }

        } else if (processList[i] > 0) { /* Consumatore */ 
                        
            /* Close the PIPE write-end */ 
            if (close(pipeDescriptor[1]) == -1) {
                perror("close");
                exit(1);
            }

            /* Write the pipe and store the value in the buffer */ 
            ssize_t size; 
            char buffer[MSG_BYTE];
            do {
                if (read(pipeDescriptor[0], &size, sizeof(size)) == -1) { // leggo dimensione chunk 
                    perror("read");
                    exit(1);
                }           
            
                if (read(pipeDescriptor[0], &buffer, size) == -1) { // leggo il contenuto del chunk 
                    perror("read");
                    exit(1);
                } 

                /* Print on STDOUT buffer content */ 
                if (write(STDOUT_FILENO, buffer, size) == -1) {
                    perror("write");
                    exit(1);
                }
            } while (size > 0);
        }
    }

    return 0;
}

