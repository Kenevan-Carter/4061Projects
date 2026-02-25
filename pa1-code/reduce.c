#include "./include/reduce.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
  // Check if the number of arguments is correct
  if (argc < 4) {
    fprintf(stderr,
            "Usage: reduce <read dir> <out file> <start ip> <end ip>\n");
    return 1;
  }
  // Check if the start IP is valid
  char *endptr = NULL;
  errno = 0;
  long start = strtol(argv[3], &endptr, 10);
  if (errno != 0 || *endptr != '\0' || start < 0 || start >= 256) {
    fprintf(stderr, "reduce: invalid IP range");
    return 1;
  }
  // Convert command line IPs from strings into integers
  // Check if the end IP is valid
  errno = 0;
  endptr = NULL;
  long end = strtol(argv[4], &endptr, 10);
  if (errno != 0 || *endptr != '\0' || end < 0 || end > 256 || end <= start) {
    fprintf(stderr, "reduce: invalid IP range");
    return 1;
  }
  // Check if the start and end IP are valid
  if (start < 0 || end > 256 || start >= end) {
    fprintf(stderr, "reduce: invalid IP range");
    return 1;
  }
  // Initialize the master table and check if it is valid
  table_t *table = table_init();
  if (table == NULL) {
    return 1;
  }
  // Open the directory and check if it is valid
  DIR *dir = opendir(argv[1]);
  if (dir == NULL) {
    perror("Failed to open directory");
    table_free(table);
    return 1;
  }
  // Read the files in the directory
  struct dirent *entry;
  char path[MAX_PATH];
  // Iterate through the files in the directory - skip the . and .. files
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    // Check if the path is valid
    // If valid - build the path to the file
    // If not valid, return an error
    if (snprintf(path, MAX_PATH, "%s/%s", argv[1], entry->d_name) >=
        (int)sizeof(path)) {
      closedir(dir);
      table_free(table);
      return 1;
    }
    // Reduce each file so it is aggregated into the master table
    // If the file is not valid, return an error
    if (reduce_file(table, path, (int)start, (int)end) != 0) {
      closedir(dir);
      table_free(table);
      return 1;
    }
  }
  // Close the directory
  closedir(dir);
  // Write the table to the output file
  if (table_to_file(table, argv[2]) != 0) {
    table_free(table);
    return 1;
  }
  // Free the table
  table_free(table);
  return 0;
}

int reduce_file(table_t *table, const char file_path[MAX_PATH],
                const int start_ip, const int end_ip) {
  // Check if the arguments are valid
  if (table == NULL || file_path == NULL || start_ip < 0 || end_ip > 256 ||
      start_ip >= end_ip) {
    fprintf(stderr, "Invalid arguments");
    return -1;
  }
  // Open the file and check if it is valid
  FILE *file = fopen(file_path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file");
    return -1;
  }
  // Allocate memory for the bucket and check if it is valid
  bucket_t *bucket = malloc(sizeof(bucket_t));
  if (bucket == NULL) {
    fprintf(stderr, "Failed to allocate memory for bucket");
    fclose(file);
    free(bucket);
    return -1;
  }
  // Read the file and aggregate the data into the master table
  while (fread(bucket, sizeof(bucket_t), 1, file) == 1) {
    int first = -1;
    // Check if the IP is valid
    if (sscanf(bucket->ip, "%d", &first) != 1) {
      continue;
    }
    // Check if the IP is in the range
    if (first < start_ip || first >= end_ip) {
      continue;
    }
    // Check if the IP is in the range
    if (first < 0 || first >= 256) {
      continue;
    }
    // Check if the IP is already in the table
    // If so, add the requests to the existing bucket
    bucket_t *existing_bucket = table_get(table, bucket->ip);
    if (existing_bucket != NULL) {
      existing_bucket->requests += bucket->requests;
    } else {
      // If bucket is not in the table, create a new bucket and add it to the
      // table If the new bucket is not valid, return an error
      bucket_t *new_bucket = bucket_init(bucket->ip);
      if (new_bucket == NULL) {
        fprintf(stderr, "Failed to allocate memory for new bucket");
        fclose(file);
        free(bucket);
        return -1;
      }
      // Set the requests for the new bucket
      new_bucket->requests = bucket->requests;
      // Add the new bucket to the table
      // If the new bucket is not valid, return an error
      if (table_add(table, new_bucket) != 0) {
        fprintf(stderr, "Failed to add new bucket to table");
        fclose(file);
        free(bucket);
        free(new_bucket);
        return -1;
      }
    }
    // Check if there is an error reading the file
    // If so, return an error
    if (ferror(file)) {
      fprintf(stderr, "Error reading file");
      fclose(file);
      free(bucket);
      return -1;
    }
  }
  // Close the file
  fclose(file);
  // Free the bucket
  free(bucket);
  return 0;
}
