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
    // Check if the number of arguments is correct
    if (argc < 4) {
        fprintf(stderr, "Usage: mapreduce <directory> <n mappers> <n reducers>\n");
        return EXIT_FAILURE;
    }
    // Convert the number of mappers and reducers to integers
    int num_mappers = atoi(argv[2]);
    int num_reducers = atoi(argv[3]);
    // Check if the number of mappers and reducers is valid
    if (num_mappers<=0 || num_reducers<=0){
        fprintf(stderr, "mapreduce: cannot have less than one mapper or reducer\n");
        return EXIT_FAILURE;
    }
    // Open the file directory and check if it is valid
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        fprintf(stderr, "opendir: No such file or directory\n");
        return EXIT_FAILURE;
    }
    // Evenly split files in directory
    struct dirent *entry;
    char *files[1000];
    int count = 0;
    // Iterate through the files in the directory
    // Skip the . and .. files
    while((entry = readdir(dir)) != NULL) {
       if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;
        // Build the full path to the file
        char full_path[512];
        if (snprintf(full_path, sizeof(full_path), "%s/%s", argv[1], entry->d_name) >=
            (int)sizeof(full_path)) {
            // if the directory is too long, close the directory and return an error
            closedir(dir);
            return EXIT_FAILURE;
        }
        // Duplicate the full path to the file
        files[count] = strdup(full_path);
        // Check if the file is valid
        if (files[count] == NULL) {
            closedir(dir);
            return EXIT_FAILURE;
        }
        // Increment the count of files
        count++;
    }
    // Close the file directory
    closedir(dir);


    // Assign files to mappers
    int files_per_mapper = count / num_mappers;
    int remainder = count % num_mappers;
    // Iterate through the mappers
    for (int i = 0; i < num_mappers; i++) {
        // Fork a new process
        pid_t pid = fork();
        // Check if the process is a child
        if (pid == 0) {
            // Calculate the start and end of the files for the mapper
            int start = i * files_per_mapper;
            int end = start + files_per_mapper;
            if (i == num_mappers - 1)
                end += remainder;
            // Build the output file name
            char outfile[256];
            snprintf(outfile, sizeof(outfile), "./intermediate/%d.tbl", i);
            int mapper_argc = (end - start) + 3;
            // Allocate memory for the arguments
            char **map_args = malloc(sizeof(char *) * mapper_argc);
            if (map_args == NULL) {
                exit(1);
            }
            // Set the arguments
            map_args[0] = "./map";
            map_args[1] = outfile;
            int arg_idx = 2;
            // Iterate through the files and set the arguments
            for (int j = start; j < end; j++) {
                map_args[arg_idx++] = files[j];
            }
            map_args[arg_idx] = NULL;
            // Execute the map program
            execv("./map", map_args);
            perror("execv failed");
            exit(1);
        }
        // Check if the process is a parent
        else if (pid == -1) {
            // Print an error message
            fprintf(stderr, "Failed to fork reducer process\n");
            return EXIT_FAILURE;
        }
    }

    // Wait for mappers to finish
    for( int i =0; i< num_mappers; i++) {
        // Wait for the mapper to finish
        int status;
        if (wait(&status) < 0) {
            return EXIT_FAILURE;
        }
    }
    // Calculate the range of IP addresses for the reducers
    int reduce_range = 256 / num_reducers;
    int red_remainder = 256 % num_reducers;
    // Iterate through the reducers
    for (int i = 0; i < num_reducers; i++) {
        // Calculate the start and end of the IP addresses for the reducer
        int start = i * reduce_range;
        int end = start + reduce_range;
        if (i == num_reducers - 1)
            end += red_remainder;
        // Fork a new process
        pid_t pid = fork();
        if (pid == 0) {
            char start_str[16], end_str[16], outfile[256];
            // Build the output file name
            snprintf(start_str, sizeof(start_str), "%d", start);
            snprintf(end_str, sizeof(end_str), "%d", end);
            snprintf(outfile, sizeof(outfile), "./out/%d.tbl", i);
            // Execute the reduce program
            execv("./reduce", (char *[]){
                "./reduce",
                "./intermediate",  // read dir
                outfile,           // out file
                start_str,         // start ip
                end_str,           // end ip
                NULL
            });
            // Print an error message
            perror("execv failed");
            exit(1);
        } else if (pid == -1) {
            // Print an error message
            fprintf(stderr, "Failed to fork reducer process\n");
            return EXIT_FAILURE;
        }
    }

    // Wait for reducers to finish
    for (int i = 0; i < num_reducers; i++) {
        int red_status;
        if (wait(&red_status) < 0) {
            return EXIT_FAILURE;
        }
        // Check if the reducer failed
        if (WIFEXITED(red_status) && WEXITSTATUS(red_status) != 0) {
            fprintf(stderr, "Reducer failed: status = %d\n", WEXITSTATUS(red_status));
        }
    }
    // Open the output directory and check if it is valid
    DIR *out_dir = opendir("./out");
    if (out_dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *out_entry;
    char out_path[MAX_PATH];

    // Iterate through the files in the output directory
    while ((out_entry = readdir(out_dir)) != NULL) {
        // Skip the . and .. files
        if (strcmp(out_entry->d_name, ".") == 0 || strcmp(out_entry->d_name, "..") == 0) {
            continue;
        }
        // Build the path to the file
        if (snprintf(out_path, sizeof(out_path), "./out/%s", out_entry->d_name) >= (int)sizeof(out_path)) {
            closedir(out_dir);
            return EXIT_FAILURE;
        }
        // Read the file and print the table
        table_t *t = table_from_file(out_path);
        if (t == NULL) {
            closedir(out_dir);
            return EXIT_FAILURE;
        }
        // Print the table
        table_print(t);
        table_free(t);
    }
    // Close the output directory
    closedir(out_dir);
    // Free the files
    for (int i = 0; i < count; i++) {
        free(files[i]);
    }
    return 0;
}
