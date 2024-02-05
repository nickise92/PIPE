/* Esercizio 3:
 * Implementare tramite system call la seguente catena di comandi script:
 * a) ls -al | sort
 * b) ls -al | cut -b27- | sort -n
 **/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char * argv[]) {
    /* Checking command line arguments: '-a' to perform 'ls -al | sort'
     * and '-b' to perform 'ls -al | cut -b27- | sort -n' */
    if (argc == 1) {
        printf("ERROR: Missing argument!\n");
        printf("USAGE: %s <arg>\n", argv[0]);
        printf("ARGUMENTS: -a, -b.\n\t-a: to perform 'ls -al | sort'.\n\t-b: to perform 'ls -al | cut -b27- | sort -n'.\n");
        exit(EXIT_SUCCESS);
    }

    if (strcmp(argv[1], "-a") == 0) {
        /* ls -al | sort */

        /* Create a pipe */
        int pipeDesc[2];
        if (pipe(pipeDesc) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }

        /* Create first process */
        pid_t pid1, pid2;
        pid1 = fork();
        if (pid1 == -1) {
            perror("Fork process 1 failed");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) { /* Child proccess */
            /* Duplicate the write-end of the pipe to STDOUT */
            if (dup2(pipeDesc[1], STDOUT_FILENO) == -1) {
                perror("dup pipe descriptor failed");
                exit(EXIT_FAILURE);
            }

            close(pipeDesc[0]);
            close(pipeDesc[1]);

            /* Exec the ls -al command, it will write to the duplicate
             * pipe descriptor that replace the STDOUT file descriptor.*/
            if (execlp("ls", "ls", "-al", NULL) == -1) {
                perror("execl ls failed");
                exit(EXIT_FAILURE);
            }
        }


        if ((pid2 = fork()) == -1) {
            perror("dup pipe descriptor failed");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) {
            /* Duplicate the read-end of the pipe into the STDIN */
            if (dup2(pipeDesc[0], STDIN_FILENO) == -1) {
                perror("dup pipe descriptor failed");
                exit(EXIT_FAILURE);
            }

            close(pipeDesc[0]);
            close(pipeDesc[1]);

            /* Exec the store command, it will read from the pipe instead
             * of the STDIN and execute the 'sort' on the previous process
             * outputs */
            if (execlp("sort", "sort", NULL) == -1) {
                perror("execl sort failed");
                exit(EXIT_FAILURE);
            }
        }

        close(pipeDesc[0]);
        close(pipeDesc[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }

    if (strcmp(argv[1], "-b") == 0) {
        /* ls -al | cut -b27- | sort -n */

        /* Create two pipes */
        int pipeDesc1[2];
        if (pipe(pipeDesc1) == -1) {
            perror("pipe failed on pipeDesc1");
            exit(EXIT_FAILURE);
        }
        int pipeDesc2[2];
        if (pipe(pipeDesc2) == -1) {
            perror("pipe failed on pipeDesc2");
            exit(EXIT_FAILURE);
        }

        /* Create processes for execute the command */
        pid_t pid1, pid2, pid3;
        /* First process: ls -al */
        if ((pid1 = fork()) == -1) {
            perror("Fork first process failed");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) { /* Child process 1 >> ls -al */
            /* Duplicate the pipe descriptor 1 (write-end) to STDOUT */
            if (dup2(pipeDesc1[1], STDOUT_FILENO) == -1) {
                perror("dup failed on pipeDesc1");
                exit(EXIT_FAILURE);
            }

            if (close(pipeDesc1[0]) == -1 || close(pipeDesc1[1]) == -1) {
                perror("close pipe descriptor failed");
                exit(EXIT_FAILURE);
            }

            /* Exec the ls -al command, it will write to the duplicate
             * pipe descriptor that replace the STDOUT file descriptor.*/
            if (execlp("ls", "ls", "-al", NULL) == -1) {
                perror("execl ls failed");
                exit(EXIT_FAILURE);
            }
        }

        if ((pid2 = fork()) == -1) {
            perror("Fork second process failed");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) { /* Child process 2 >>  cut -n27- */
            /* Duplicate the pipe descriptor 1 (read-end) to STDIN */
            if (dup2(pipeDesc1[0], STDIN_FILENO) == -1) {
                perror("dup failed on pipeDesc1");
                exit(EXIT_FAILURE);
            }

            if (close(pipeDesc1[0]) == -1 || close(pipeDesc1[1]) == -1) {
                perror("close pipe descriptor failed");
                exit(EXIT_FAILURE);
            }
            /* Duplicate pipe descriptor 2 (write-end) to STDOUT */
            if (dup2(pipeDesc2[1], STDOUT_FILENO) == -1) {
                perror("dup failed on pipeDesc1");
                exit(EXIT_FAILURE);
            }

            if (close(pipeDesc2[0]) == -1 || close(pipeDesc2[1]) == -1) {
                perror("close pipe descriptor failed");
                exit(EXIT_FAILURE);
            }

            /* Execute the cut -b27- command, it will read the input from the pipe
             * duplicated into file descriptor STDOUT.
             * It will write to the second duplicate pipe descriptor that replace
             * the STDOUT file descriptor.*/
            if (execlp("cut", "cut", "-b27-", NULL) == -1) {
                perror("execl cut failed");
                exit(EXIT_FAILURE);
            }
        }

        if ((pid3 = fork()) == -1) {
            perror("fork process 3 failed");
            exit(EXIT_FAILURE);
        }

        if (pid3 == 0) {
            /* Duplicate the pipe descriptor 2 (read-end) to STDIN */
            if(dup2(pipeDesc2[0], STDIN_FILENO) == -1) {
                perror("dup pipeDesc2 failed");
                exit(EXIT_FAILURE);
            }

            if (close(pipeDesc1[0]) == -1 || close(pipeDesc1[1]) == -1) {
                perror("close pipe descriptor failed");
                exit(EXIT_FAILURE);
            }
            if (close(pipeDesc2[0]) == -1 || close(pipeDesc2[1]) == -1) {
                perror("close pipe descriptor failed");
                exit(EXIT_FAILURE);
            }

            /* Execute the sort -n command. It will read from the pipeDesc2 (STDIN)
             * and print the output in the terminal */
            if (execlp("sort", "sort", NULL) == -1) {
                perror("execl sort failed");
                exit(EXIT_FAILURE);
            }

        }

        if (close(pipeDesc1[0]) == -1 || close(pipeDesc1[1]) == -1) {
            perror("close pipeDesc1 failed");
            exit(EXIT_FAILURE);
        }
        if (close(pipeDesc2[0]) == -1 || close(pipeDesc2[1]) == -1) {
            perror("close pipeDesc1 failed");
            exit(EXIT_FAILURE);
        }

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        waitpid(pid3, NULL, 0);

    }



    return 0;
}