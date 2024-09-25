#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>   // For directory operations
#include <ctype.h>    // For checking if a character is a digit

#define TARGET_PROCESS "gedit"  // Target process to check

// Function to check if gedit is running
int printoutProccesses() {
    int test = 0;

    struct dirent *entry; // hold an info on directory entry 
    DIR *dp = opendir("/proc"); // create pointer to proc stream

    if(dp == NULL){
        perror("Error: ccouldn't open /proc");
        return 0;
    }

    while ((entry = readdir(dp)) != NULL){ // read every entry in /proc
        // check if entry is numerical 
        if (isdigit(*entry->d_name)){
            char filepath[256];
            char proccess_name[256];

            // print full path to file path 
            snprintf(filepath, sizeof(filepath), "/proc/%s/comm", entry->d_name);
            // open and read file
            FILE *fp = fopen(filepath, "r");

            if (fp != NULL){
                // read proces name
                if (fgets(proccess_name, sizeof(proccess_name), fp) != NULL){
                    printf("Name: %s\n", proccess_name);
                }
            }
        }
    }



    return 0;  // gedit is not running
}

int main() {

    printoutProccesses();


    return 0;
}
