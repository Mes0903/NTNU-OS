#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define NUM_ZOMBIES 2000

int main(void) {
    pid_t pid;
    int i;
    
    // Print parent's PID so you know which process to target.
    printf("Parent PID: %d\n", getpid());
    
    for (i = 0; i < NUM_ZOMBIES; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Child process exits immediately to become a zombie.
            exit(0);
        }
        // Parent continues the loop without waiting for the child.
    }
    
    printf("Created %d zombie processes.\n", NUM_ZOMBIES);
    printf("Press Enter to terminate the parent process...\n");
    getchar();
    
    // When the parent terminates, its zombie children become orphaned.
    // The init process will eventually reap them, proving Linux is a capable zombie killer.
    exit(EXIT_SUCCESS);
}
