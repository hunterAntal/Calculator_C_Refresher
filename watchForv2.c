#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    while(1){
        char command[64];
        strcpy(command, "ps -e | grep gedit");
        system(command);
    }
    
}   