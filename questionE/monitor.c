#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>     // For directory operations
#include <unistd.h>     // For various POSIX functions
#include <ctype.h>      // For character type functions
#include <sys/types.h>  // For data types
#include <sys/stat.h>   // For file status
#include <sys/time.h>   // For time structures
#include <signal.h>     // For signal handling

// Define thresholds for memory and CPU usage
#define MEM_THRESHOLD (200 * 1024) // 200 KB
#define CPU_THRESHOLD (3 * 60) // CPU time in s (3 min)

typedef struct {
    pid_t pid; // process ID
    char exeName[256]; // process name
    unsigned long memUsage; // memory usage in KB
    unsigned long cpuTime; // CPU time in s
    int exceedsMem; // flag for memory
    int exceedsCPU; // flag for CPU
    int number; // number associated with specific process
} ProcessInfo;



int main(){
    // infinite loop to keep listining for user input and to update terminal
    while (1){
        // Array to store processes that exceed thresholds
        ProcessInfo procList[1024];
        int procCount = 0;  // Count of processes stored
        int number = 1;     // Number assigned to each process for user reference
        // opening the /proc directory 
        DIR *procDIr = opendir("/proc");
        struct dirent *entry;

        // check that the directory was available 
        if (procDIr == NULL){
            perror("Failed to open /proc");
            return 1;
        }

        // Read directory stream to entry
        while ((entry = readdir(procDIr)) != NULL){
            // skip the entries that do not have pid aka start with digit
            if(!isdigit(entry->d_name[0])){
                continue;
            }

            // record the pid by forcing the string to int
            pid_t pid = atoi(entry->d_name);

            // ** Creating File Paths **
            char statPath[256], cmdlinePath[256], statusPath[256];
            char buffer[1024];

            sprintf(statPath, "/proc/%d/stat", pid); // contains process status info
            sprintf(cmdlinePath, "/proc/%d/cmdline", pid); // contains command line arguments
            sprintf(statusPath, "/proc/%d/status", pid); // contains humane-readable statuses

            // ** Retreieving the exe name **

            FILE *cmdlineFIle = fopen(cmdlinePath, "r");
            char exeName[256] = {0};

            if (cmdlineFIle){
                // Read the command line arguments to find process name corosponding to pid
                size_t size = fread(exeName, sizeof(char), sizeof(exeName), cmdlineFIle);
                if (size > 0){
                    exeName[size] = "\0"; // Null-terminate the string so we know where the name ends
                } else {
                    sprintf(exeName, "[%d]", pid); // If command line is empty, use the PID as the name
                }
                fclose(cmdlineFIle); // close the file
            
            } else {
                sprintf(exeName, "[%d]", pid); // If unable to open cmdline file, use the PID as the name
            }

            // ** Retreieving the Memory Usage ** //

            unsigned long memUsage = 0;
            FILE *statusFile = fopen(statusPath, "r");

            if (statusFile){
                while(fgets(buffer, sizeof(buffer), statusFile)){ // go through status to find VmRSS
                    if (strncmp(buffer, "VmRSS:", 6) == 0){ // search buffer for VmRSS
                        sscanf(buffer, "VmRSS: %lu", &memUsage); // input the memory usage and break
                        break;
                    }
                }
                fclose(statusFile); // close the file 
            }

            // ** Retrieving the CPU Time ** //

            unsigned long utime = 0, stime = 0;
            FILE *statFile = fopen (statPath, "r"); // open /proc/[pid]/stat to get CPU times

            if (statFile){
                // Skip over pid(%d), comm(%s), state(%c), ppid(%d), pgrp(%d), session(%d), tty_rn(%d), tpgid(%d), flags(%u), minflt(%lu), cminflt(%lu), majflt(%lu), cmajflt(%lu)
                fscanf(statFile, 
                "%*d %*s %*c %*d %*d %*d %*d %*d " 
                "%*u %*lu %*lu %*lu %*lu %lu %lu", &utime, &stime); // IF BREAKS CHANGE BACK HERE!!!!!!
                fclose(statFile); // close the file
            }

            // Convert ticks to seconds 
            long ticksPerSec = sysconf(_SC_CLK_TCK);
            unsigned long totalTime = (utime + stime) / ticksPerSec;

            // ** Storing Processes Exceeding Thresholds ** //

            if (memUsage > MEM_THRESHOLD || totalTime > CPU_THRESHOLD){
                ProcessInfo *process = &procList[procCount++]; // create an object process and add it to the array of exceeding processes
                // fill out the info of the object
                process->pid = pid;
                strncpy(process->exeName, exeName, sizeof(process->exeName) - 1);  // leave space for the \0
                process->memUsage = memUsage;
                process->cpuTime = totalTime;
                process->exceedsMem = 1; // true
                process->exceedsCPU = 1; // treu
                process->number = number++;
            }

            // ** Disply to User ** //

            system("clear"); // clear the terminal 

            int anyMem = 0, anyCPU = 0;

            printf("Using More Than 200K:\n\n");
            for (int i = 0; i < procCount; i++) { // iterate through the list of exceeding processes
                if (procList[i].exceedsMem){ // if the process exceeds memory
                    printf("%d- %s\n", procList[i].number, procList[i].exeName);
                    anyMem = 1; // there is processes using more then 200K mem
                }
            } 
            // if there are no processes using more then 200K then display none
            if(!anyMem){
                printf("No processes using over 200K of memory.\n");
            }

            printf("Alive For More Than 3 Minutes:\n\n");
            for (int i = 0; i < procCount; i++){
                if (procList[i].exceedsCPU){ // if the process exceeds time
                    printf("%d- %s\n", procList[i].number, procList[i].exeName);
                    anyCPU = 1; // there is processes alive for more then 3 mins
                }
            }
            // if there are no processes alive for more then 3 mins
            if(!anyCPU){
                printf("No processes alive for more then 3 minutes.\n");
            }

            // ** Taking User Input ** //

            printf("\nEnter a number to kill, or press Enter to refresh -> ");
            char input[256];
            fgets(input, sizeof(input), stdin); // Read user input

            if (input[0] != '\n') { // Check if the user entered something
            int num = atoi(input); // Convert input to an integer

            // Find the process with the corresponding number
            ProcessInfo *pi = NULL;
            for (int i = 0; i < procCount; ++i) {
                if (procList[i].number == num) {
                    pi = &procList[i];
                    break;
                }
            }

            if (pi != NULL) {
                // Attempt to terminate the process using SIGTERM signal
                if (kill(pi->pid, SIGTERM) == 0) {
                    printf("Process %s terminated.\n", pi->exeName);
                } else {
                    perror("Failed to terminate process"); // Print error if kill fails
                }
            } else {
                printf("Invalid number entered.\n");
            }
            printf("Press Enter to continue...");
            fgets(input, sizeof(input), stdin); // Wait for user to press Enter
        }

        // Sleep for 5 seconds before refreshing
        sleep(5);

        }
    }
    
    return 0;
}