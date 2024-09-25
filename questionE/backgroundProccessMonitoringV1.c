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
#define MEM_THRESHOLD (200 * 1024) // Memory threshold in KB (200 KB)
#define CPU_THRESHOLD (3 * 60)     // CPU time threshold in seconds (3 minutes)

// Structure to hold process information
typedef struct {
    pid_t pid;               // Process ID
    char exeName[256];       // Executable name
    unsigned long memUsage;  // Memory usage in KB
    unsigned long cpuTime;   // CPU time in seconds
    int exceedsMem;          // Flag indicating memory threshold exceeded
    int exceedsCPU;          // Flag indicating CPU threshold exceeded
    int number;              // Assigned number for user selection
} ProcessInfo;

// Function prototypes
unsigned long get_total_cpu_time();

int main() {
    while (1) {
        // Array to store processes that exceed thresholds
        ProcessInfo procList[1024];
        int procCount = 0;  // Count of processes stored
        int number = 1;     // Number assigned to each process for user reference

        // Open the /proc directory to read process information
        DIR *procDir = opendir("/proc");
        struct dirent *entry; // Structure representing directory entries

        if (procDir == NULL) {
            perror("opendir"); // Print error message if opening fails
            return 1;
        }

        // Read each entry in the /proc directory
        while ((entry = readdir(procDir)) != NULL) {
            // Skip entries that are not process directories (which are numeric)
            if (!isdigit(entry->d_name[0]))
                continue;

            // Convert the directory name to a PID
            pid_t pid = atoi(entry->d_name);

            // Paths to various files within the /proc/[pid] directory
            char statPath[256], cmdlinePath[256], statusPath[256];
            FILE *statFile, *cmdlineFile, *statusFile;
            char buffer[1024];

            // Construct the file paths
            sprintf(statPath, "/proc/%d/stat", pid);       // Contains process status
            sprintf(cmdlinePath, "/proc/%d/cmdline", pid); // Contains the command line arguments
            sprintf(statusPath, "/proc/%d/status", pid);   // Contains process status in human-readable form

            // Get the command name of the process
            cmdlineFile = fopen(cmdlinePath, "r");
            char exeName[256] = {0}; // Initialize the executable name buffer

            if (cmdlineFile) {
                // Read the command line arguments
                size_t size = fread(exeName, sizeof(char), sizeof(exeName), cmdlineFile);
                if (size > 0) {
                    exeName[size] = '\0'; // Null-terminate the string
                } else {
                    // If command line is empty, use the PID as the name
                    sprintf(exeName, "[%d]", pid);
                }
                fclose(cmdlineFile); // Close the file
            } else {
                // If unable to open cmdline file, use the PID as the name
                sprintf(exeName, "[%d]", pid);
            }

            // Get memory usage of the process
            unsigned long memUsage = 0; // Initialize memory usage

            statusFile = fopen(statusPath, "r");
            if (statusFile) {
                // Read the status file line by line
                while (fgets(buffer, sizeof(buffer), statusFile)) {
                    // Look for the line starting with "VmRSS:"
                    if (strncmp(buffer, "VmRSS:", 6) == 0) {
                        // Extract the memory usage value
                        sscanf(buffer, "VmRSS: %lu", &memUsage); // memUsage is in KB
                        break;
                    }
                }
                fclose(statusFile); // Close the file
            }

            // Get CPU time of the process
            unsigned long utime = 0, stime = 0; // User and system CPU times

            statFile = fopen(statPath, "r");
            if (statFile) {
                // The /proc/[pid]/stat file contains various statistics in a single line
                // We are interested in fields 14 (utime) and 15 (stime)
                // Use a format string to skip unwanted fields
                fscanf(statFile,
                       "%*d %*s %*c %*d %*d %*d %*d %*d "
                       "%*u %*u %*u %*u %*u %lu %lu",
                       &utime, &stime);
                fclose(statFile); // Close the file
            }

            // Convert clock ticks to seconds for CPU time
            long ticks_per_sec = sysconf(_SC_CLK_TCK); // Get number of clock ticks per second
            unsigned long totalTime = (utime + stime) / ticks_per_sec; // Total CPU time in seconds

            // Check if the process exceeds memory or CPU thresholds
            int exceedsMem = memUsage > MEM_THRESHOLD;
            int exceedsCPU = totalTime > CPU_THRESHOLD;

            if (exceedsMem || exceedsCPU) {
                // Store process information in the procList array
                ProcessInfo *pi = &procList[procCount++]; // Get pointer to next available slot
                pi->pid = pid; // Store process ID
                strncpy(pi->exeName, exeName, sizeof(pi->exeName) - 1); // Store executable name
                pi->memUsage = memUsage; // Store memory usage
                pi->cpuTime = totalTime; // Store CPU time
                pi->exceedsMem = exceedsMem; // Set memory threshold flag
                pi->exceedsCPU = exceedsCPU; // Set CPU threshold flag
                pi->number = number++; // Assign a number for user selection
            }
        }
        closedir(procDir); // Close the /proc directory

        // Display the processes that exceed thresholds
        system("clear"); // Clear the console screen (optional)

        int anyMem = 0, anyCPU = 0; // Flags to check if any process exceeds thresholds

        // Display processes exceeding memory threshold
        printf("Using more than 200K:\n\n");
        for (int i = 0; i < procCount; ++i) {
            if (procList[i].exceedsMem) {
                printf("%d- %s (PID %d)\n", procList[i].number, procList[i].exeName, procList[i].pid);
                anyMem = 1; // Set flag indicating at least one process exceeds memory threshold
            }
        }
        if (!anyMem) {
            printf("None\n");
        }

        // Display processes exceeding CPU time threshold
        printf("\nUsing more than 3 minutes:\n\n");
        for (int i = 0; i < procCount; ++i) {
            if (procList[i].exceedsCPU) {
                printf("%d- %s (PID %d)\n", procList[i].number, procList[i].exeName, procList[i].pid);
                anyCPU = 1; // Set flag indicating at least one process exceeds CPU threshold
            }
        }
        if (!anyCPU) {
            printf("None\n");
        }

        // Prompt the user for input
        printf("\nEnter a number to kill, or wait for the next refresh -> ");
        fflush(stdout); // Flush the output buffer to ensure the prompt is displayed

        // Set up for non-blocking user input with a timeout
        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);              // Clear the set
        FD_SET(STDIN_FILENO, &set); // Add stdin file descriptor to the set

        timeout.tv_sec = 1;         // Set timeout to 1 second
        timeout.tv_usec = 0;

        // Use select to wait for input on stdin with a timeout
        int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

        if (rv > 0) {
            // There is data available to read
            char input[256];
            if (fgets(input, sizeof(input), stdin) != NULL) {
                // Convert the input to an integer
                int num = atoi(input);
                if (num > 0) {
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
                            printf("Process %s (PID %d) terminated.\n", pi->exeName, pi->pid);
                        } else {
                            perror("Failed to terminate process"); // Print error if kill fails
                        }
                        printf("Press Enter to continue...");
                        getchar(); // Wait for user to press Enter
                    } else {
                        // Invalid number entered
                        printf("Invalid number entered.\n");
                        printf("Press Enter to continue...");
                        getchar(); // Wait for user to press Enter
                    }
                }
            }
        }

        // Sleep for a short duration to reduce CPU usage
        usleep(100000); // Sleep for 100 milliseconds
    }
    return 0;
}
