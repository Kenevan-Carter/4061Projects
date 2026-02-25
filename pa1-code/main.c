#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./include/table.h"
#include "./include/map.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: mapreduce <directory> <n mappers> <n reducers>\n");
        return EXIT_FAILURE;
    }
    int num_mappers = atoi(argv[2]);
    int num_reducers = atoi(argv[3]);
    if (num_mappers<=0 || num_reducers<=0){
        fprintf(stderr, "mapreduce: cannot have less than one mapper or reducer\n");
        return EXIT_FAILURE;
    }
    // initialize pointer to the file directory
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("Failed to open directory");
        return EXIT_FAILURE;
    }
    //evenly split files in directory
    struct dirent *entry;
    char *files[1000];
    int count = 0;
    while((entry = readdir(dir)) != NULL) {
       if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;
        files[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);


    //assign files to mappers
    int files_per_mapper = count / num_mappers;
    int remainder = count % num_mappers;
    printf("Total files: %d, Files per mapper: %d, Remainder: %d\n", count, files_per_mapper, remainder);
    for (int i = 0; i < num_mappers; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            int start = i * files_per_mapper;
            int end = start + files_per_mapper;
            if (i == num_mappers - 1)
                end += remainder;
            char outfile[256];
            snprintf(outfile, sizeof(outfile), "./intermediate/%d_table.tbl", i);
            for (int j = start; j < end; j++) {
                execv("./map", (char *[]){
                    "./map",
                    outfile,  
                    files[j],    
                    NULL
                });
            }
            perror("execv failed");
            exit(1);
        } 
        else if (pid == -1) {
            fprintf(stderr, "Failed to fork reducer process\n");
            return EXIT_FAILURE;
        }
    }

    //wait for mappers to finish
    for( int i =0; i< num_mappers; i++) {

        int status;
        pid_t finished = wait(&status);

        if(WEXITSTATUS(status) != 0){
            fprintf(stderr,"mapper failed: status = %d\n", WEXITSTATUS(status));
        }
    }
    int reduce_range = 256 / num_reducers;
    int red_remainder = 256 % num_reducers;
    for (int i = 0; i < num_reducers; i++) {
        int start = i * reduce_range;
        int end = start + reduce_range;
        if (i == num_reducers - 1)
            end += red_remainder;

        pid_t pid = fork();
        if (pid == 0) {
            char start_str[16], end_str[16], outfile[256];
            snprintf(start_str, sizeof(start_str), "%d", start);
            snprintf(end_str, sizeof(end_str), "%d", end);
            snprintf(outfile, sizeof(outfile), "./intermediate/reduce_%d.tbl", i);

            execv("./reduce", (char *[]){
                "./reduce",
                "./intermediate",  // read dir
                outfile,           // out file
                start_str,         // start ip
                end_str,           // end ip
                NULL
            });
            perror("execv failed");
            exit(1);
        } else if (pid == -1) {
            fprintf(stderr, "Failed to fork reducer process\n");
            return EXIT_FAILURE;
        }
    }

    //wait for reducers to finish
    for (int i = 0; i < num_reducers; i++) {
        int red_status;
        if (WEXITSTATUS(red_status) != 0) {
            fprintf(stderr, "Reducer failed: status = %d\n", WEXITSTATUS(red_status));
        }
    }
    return 0;
}
