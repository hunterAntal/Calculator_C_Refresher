#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

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

        }
    }
    
    return 0;
}