#include "./include/map.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: map <outfile> <infiles...>\n");
        return EXIT_FAILURE;
    }
    FILE *fd = fopen(argv[1], "w");
    if (fd == NULL) {
        perror("Failed to open output file\n");
        return EXIT_FAILURE;
    }

    // iterate over infiles starting at argv[2]
    table_t *table = table_init();
    for (int i = 2; i < argc; i++) {
        map_log(table, argv[i]);
    }
    table_to_file(table, argv[1]);
    table_free(table);
    fclose(fd);
    return 0;
}


int map_log(table_t *table, const char file_path[MAX_PATH]) {
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        perror("Failed to open input file\n");
        return -1;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *ip = strtok(line, ",");  // date
        ip = strtok(NULL, ",");         // ip address
        table_add(table, bucket_init(ip));
    }
    fclose(f);
    return 0;
}
