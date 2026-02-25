#include "./include/map.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
  // Check if the number of arguments is correct
  if (argc < 3) {
    fprintf(stderr, "Usage: map <outfile> <infiles...>\n");
    return EXIT_FAILURE;
  }
  // Initialize the table and check if it is valid
  table_t *table = table_init();
  // Iterate through the files and map the logs
  for (int i = 2; i < argc; i++) {
    // Map the logs and check if it is valid
    if (map_log(table, argv[i]) != 0) {
      table_free(table);
      return EXIT_FAILURE;
    }
  }
  // Write the table to the output file and free the table
  table_to_file(table, argv[1]);
  table_free(table);
  return 0;
}

int map_log(table_t *table, const char file_path[MAX_PATH]) {
  // Open the file and check if it is valid
  FILE *f = fopen(file_path, "r");
  if (f == NULL) {
    perror("fopen: No such file or directory");
    return -1;
  }
  // Allocate memory for the line
  char line[256];
  // Iterate through the lines in the file getting the IP address
  while (fgets(line, sizeof(line), f)) {
    char *ip = strtok(line, ","); // date
    ip = strtok(NULL, ",");       // ip address
    bucket_t *existing = table_get(table, ip);
    if (existing != NULL) {
      existing->requests++;
      // If the bucket is not valid, create a new bucket and add it to the table
    } else {
      // Create a new bucket and add it to the table
      bucket_t *b = bucket_init(ip);
      // Check if the bucket is valid
      if (b == NULL)
        return -1;
      b->requests = 1;
      // Add the bucket to the table and check if it is valid
      if (table_add(table, b) != 0) {
        free(b);
        return -1;
      }
    }
  }
  // Close the file
  fclose(f);
  return 0;
}
