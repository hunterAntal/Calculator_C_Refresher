// This is question D
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>     // For handling signals like SIGKILL
#include <unistd.h>     // For system calls like fork, exec, sleep



int main(void){
    char command[64];
    strcpy(command, "kill $(pgrep gedit)");
    while(1){
        system(command);
        sleep(3);
    }
    
}   