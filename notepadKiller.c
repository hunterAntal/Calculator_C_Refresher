// Hunter Antal
// 1181729
// 3655 Project 1
// Task 2: (C)

#include <stdio.h>      // For input/output functions like printf
#include <stdlib.h>     // For general utilities like exit
#include <unistd.h>     // For system calls like fork, exec, sleep
#include <sys/types.h>  // For defining data types used in system calls (e.g., pid_t)
#include <signal.h>     // For handling signals like SIGKILL


int main(){

    // using fork sys call to create a child process
    // pid = 0 if successful 
    // pid = -1 if fail
    pid_t pid = fork();

    // check that fork was successful
    if (pid == 0){
        // replace the child process with gedit (my note app)
        execl("gedit", "gedit", NULL);
    } else {
        printf("Launched gedit with PID %d\n", pid);
        sleep(3); // Wait for 3 seconds
        kill(pid, SIGKILL); // Kill the process
        printf("Killed gedit with PID %d\n", pid);
    }




    return 0;
}