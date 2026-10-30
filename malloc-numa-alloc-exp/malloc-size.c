#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // Required for malloc_usable_size() on Linux/glibc
#include <string.h>

void print_memory_usage() {
    FILE* file = fopen("/proc/self/status", "r");
    char line[128];

    while (fgets(line, sizeof(line), file)) {
        // VmRSS is the actual physical RAM (Resident Set Size) being used
        if (strncmp(line, "VmRSS:", 6) == 0) {
            printf("%s", line);
            break;
        }
    }
    fclose(file);
}

int main(void) {
    size_t requested_size = 420989; // Ask for 42 bytes
    char *ptr = malloc(requested_size);

    if (ptr == NULL) {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    size_t usable_size = malloc_usable_size(ptr);

    printf("Requested bytes: %zu\n", requested_size);
    printf("Usable bytes:    %zu\n", usable_size);
    print_memory_usage();

    free(ptr);
    return EXIT_SUCCESS;
}

