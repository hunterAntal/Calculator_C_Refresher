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

            // ** Retreaving the exe name **

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

            // ** Retreieving Memory Usage ** //

            unsigned long memUsage = 0;
            FILE *statusFile = fopen(statusPath, "r");

            if (statusFile){
                while(fgets(buffer, sizeof(buffer), statusFile)){
                    if (strncmp(buffer, "VmRSS:", 6) == 0)
                }
            }

        }
    }
    
    return 0;
}