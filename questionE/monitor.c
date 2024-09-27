// Question E
// Hunter Antal
// 1181729
#include <stdio.h>      // For input/output functions like printf
#include <stdlib.h>     // For general utilities like exit
#include <string.h>     // String manipulation functions like strcpy
#include <dirent.h>     // For directory operations
#include <unistd.h>     // For various POSIX functions
#include <ctype.h>      // For character type functions
#include <sys/types.h>  // For data types
#include <sys/stat.h>   // For file status
#include <sys/time.h>   // For time structures
#include <signal.h>     // For signal handling

// Define thresholds for memory and CPU usage
#define MEM_THRESHOLD (200 * 1024) // 200 KB
#define CPU_THRESHOLD (3 * 60)     // CPU time in seconds (3 minutes)

// Structure to hold process information
typedef struct {
    pid_t pid;               // Process ID
    char exeName[256];       // Process name
    unsigned long memUsage;  // Memory usage in KB
    unsigned long cpuTime;   // CPU time in seconds
    int exceedsMem;          // Flag for memory threshold
    int exceedsCPU;          // Flag for CPU threshold
    int number;              // Number associated with the process
} ProcessInfo;

int main() {
    // Infinite loop to keep monitoring and updating
    while (1) {
        // Array to store processes that exceed thresholds
        ProcessInfo procList[1024];
        int procCount = 0;  // Count of processes stored
        int number = 1;     // Number assigned to each process for user reference

        // Open the /proc directory
        DIR *procDir = opendir("/proc");
        struct dirent *entry;

        // Check that the directory was opened successfully
        if (procDir == NULL) {
            perror("Failed to open /proc");
            return 1;
        }

        // Read directory entries
        while ((entry = readdir(procDir)) != NULL) {
            // Skip entries that are not process directories (which are numeric)
            if (!isdigit(entry->d_name[0])) {
                continue;
            }

            // Convert the directory name to a PID
            pid_t pid = atoi(entry->d_name);

            // Creating file paths
            char statPath[256], cmdlinePath[256], statusPath[256];
            char buffer[1024];

            sprintf(statPath, "/proc/%d/stat", pid);       // Contains process status info
            sprintf(cmdlinePath, "/proc/%d/cmdline", pid); // Contains command line arguments
            sprintf(statusPath, "/proc/%d/status", pid);   // Contains human-readable statuses

            // Retrieving the executable name
            FILE *cmdlineFile = fopen(cmdlinePath, "r");
            char exeName[256] = {0};

            if (cmdlineFile) {
                // Read the command line arguments to find process name corresponding to PID
                size_t size = fread(exeName, sizeof(char), sizeof(exeName) - 1, cmdlineFile);
                exeName[size] = '\0'; // Null-terminate the string
                fclose(cmdlineFile);  // Close the file
            } else {
                sprintf(exeName, "[%d]", pid); // Use PID as name if cmdline cannot be read
            }

            // Retrieving the memory usage
            unsigned long memUsage = 0;
            FILE *statusFile = fopen(statusPath, "r");

            if (statusFile) {
                while (fgets(buffer, sizeof(buffer), statusFile)) {
                    // Search for the line starting with "VmRSS:"
                    if (strncmp(buffer, "VmRSS:", 6) == 0) {
                        sscanf(buffer, "VmRSS: %lu", &memUsage); // Extract memory usage
                        break;
                    }
                }
                fclose(statusFile); // Close the file
            }

            // Retrieving the CPU time
            unsigned long utime = 0, stime = 0;
            FILE *statFile = fopen(statPath, "r"); // Open /proc/[pid]/stat

            if (statFile) {
                // Skip fields to reach utime and stime (fields 14 and 15)
                fscanf(statFile,
                       "%*d %*s %*c %*d %*d %*d %*d %*d "
                       "%*u %*u %*u %*u %*u %lu %lu",
                       &utime, &stime);
                fclose(statFile); // Close the file
            }

            // Convert ticks to seconds
            long ticksPerSec = sysconf(_SC_CLK_TCK);
            unsigned long totalTime = (utime + stime) / ticksPerSec;

            // Check if the process exceeds thresholds
            int exceedsMem = memUsage > MEM_THRESHOLD ? 1 : 0;
            int exceedsCPU = totalTime > CPU_THRESHOLD ? 1 : 0;

            if (exceedsMem || exceedsCPU) {
                // Store process information
                ProcessInfo *process = &procList[procCount++]; // Add to array
                process->pid = pid;
                strncpy(process->exeName, exeName, sizeof(process->exeName) - 1); // Copy name
                process->exeName[sizeof(process->exeName) - 1] = '\0';            // Ensure null-terminated
                process->memUsage = memUsage;
                process->cpuTime = totalTime;
                process->exceedsMem = exceedsMem;
                process->exceedsCPU = exceedsCPU;
                process->number = number++;
            }
        }
        closedir(procDir); // Close the /proc directory

        // Display to user
        system("clear"); // Clear the terminal

        int anyMem = 0, anyCPU = 0;

        printf("Using More Than 200K:\n\n");
        for (int i = 0; i < procCount; i++) {
            if (procList[i].exceedsMem) {
                printf("%d- %s\n", procList[i].number, procList[i].exeName);
                anyMem = 1; // There are processes using more than 200K memory
            }
        }
        if (!anyMem) {
            printf("No processes using over 200K of memory.\n");
        }

        printf("\nAlive For More Than 3 Minutes:\n\n");
        for (int i = 0; i < procCount; i++) {
            if (procList[i].exceedsCPU) {
                printf("%d- %s\n", procList[i].number, procList[i].exeName);
                anyCPU = 1; // There are processes alive for more than 3 minutes
            }
        }
        if (!anyCPU) {
            printf("No processes alive for more than 3 minutes.\n");
        }

        // Taking user input
        printf("\nEnter a number to kill, or Wait 5 Seconds for refresh -> ");
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
                // Attempt to terminate the process using SIGKILL signal
                if (kill(pi->pid, SIGKILL) == 0) {
                    printf("Process %s terminated.\n", pi->exeName);
                } else {
                    perror("Failed to terminate process"); // Print error if kill fails
                }
            } else {
                printf("Invalid number entered.\n");
            }
            printf("Please wait 5 Seconds for Refresh\n");
            
        }

        // Sleep for 5 seconds before refreshing
        sleep(5);
    }

    return 0;
}
