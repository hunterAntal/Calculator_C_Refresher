#include <stdio.h>      // For input/output functions like printf
#include <stdlib.h>     // For general utilities like exit
#include <unistd.h>     // For system calls like fork, exec, sleep
#include <sys/types.h>  // For defining data types used in system calls (e.g., pid_t)
#include <signal.h>     // For handling signals like SIGKILL
#include <sys/wait.h>   // For waitpid

int main() {

    // using fork sys call to create a child process
    // pid = 0 if successful 
    // pid = -1 if fail
    pid_t pid = fork();

    if (pid < 0) {
        printf("Error: Fork failed\n");
        exit(1); // Exit with error code
    }
    // check that fork was successful
    else if (pid == 0) {
        // In the child process: replace the child process with gedit (my note app)
        execl("/usr/bin/gedit", "gedit", NULL);

        // If execl fails
        printf("Error: Failed to launch gedit\n");
        exit(1);
    } else {
        // Parent process
        printf("Launched gedit with PID %d\n", pid);
        sleep(3); // Wait for 3 seconds

        // Kill the child process running gedit
        kill(pid, SIGKILL);
        printf("Killed gedit with PID %d\n", pid);

        // Wait for the child process to terminate properly
        waitpid(pid, NULL, 0);
    }

    return 0;
}
