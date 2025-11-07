#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_process_status(int pid) {
    char filepath[256];
    sprintf(filepath, "/proc/%d/status", pid);
    FILE *file = fopen(filepath, "r");
    if (!file) {
            fprintf(stderr, "Cannot open the status file for process %d\n", pid);
            return;
    }

    char line[1024];
    char state[10] = "";
    while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "State:", 6) == 0) {
                    sscanf(line, "State: %9s", state);
                    break;
            }
    }
    if (strlen(state) > 0) {
        printf("The state of process %d is: %s\n", pid, state);
    } else {
        printf("The state of process %d could not be determined.\n", pid);
    }
    fclose(file);
}


int main() {
        int target_pid;
        printf("Enter the target process pid:\n");
        scanf("%d", &target_pid);
        print_process_status(target_pid);
        return 0;
}
