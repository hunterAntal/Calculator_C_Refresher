// This is question D
// Hunter Antal
// 1181729
#include <stdio.h>      // Standard Input Output library
#include <string.h>     // String manipulation functions like strcpy
#include <stdlib.h>     // Standard library for functions like system
#include <signal.h>     // For handling signals like SIGKILL
#include <unistd.h>     // For system calls like fork, exec, and sleep

int main(void) {
    char command[64];   // Declare a character array to hold the command string
    strcpy(command, "kill $(pgrep gedit)");   // Copy the kill command to terminate 'gedit' process into the command array
    
    while(1) {   // Infinite loop to repeatedly run the command
        system(command);   // Execute the system command to kill 'gedit' processes
        sleep(3);   // Pause for 3 seconds before repeating
    }
}
