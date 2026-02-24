#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./include/map.h"

bucket_t *bucket_init(const char ip[IP_LEN]) {
    if (ip == NULL){
        perror("Invalid IP");
        return NULL;
    }
    bucket_t *new_bucket = malloc(sizeof(bucket_t));
    strncpy(new_bucket->ip, ip, IP_LEN);
    new_bucket->requests = 0;
    new_bucket->next = NULL;
    return new_bucket;
}

table_t *table_init() {
    table_t *new_table = malloc(sizeof(table_t));
    if (new_table == NULL) {
        perror("Failed to allocate memory for table");
        return NULL;
    }
    memset(new_table, 0, sizeof(table_t));
    for (int i = 0; i<TABLE_LEN; i++) {
        new_table->buckets[i] = NULL;
    }
    return new_table;
}

void table_print(const table_t *table) {
    for (int i = 0; i<TABLE_LEN; i++) {
        bucket_t *current = table->buckets[i];
        while (current != NULL) {
            printf("%s - %d\n", current->ip, current->requests);
            bucket_t *next = current->next;
            current = next;
        }
    }
    printf("\n");
}

void table_free(table_t *table) {
    if (table == NULL) {
        perror("Table is NULL");
        return;
    }
    for (int i = 0; i<TABLE_LEN; i++) {
        bucket_t *current = table->buckets[i];
        while (current != NULL) {
            bucket_t *next = current->next;
            free(current);
            current = next;
        }
    }
    free(table);
}

int table_add(table_t *table, bucket_t *bucket) {
    if (table_get(table, bucket->ip) != NULL) {
        perror("Bucket already exists");
        return -1;
    }
    if (table == NULL || bucket == NULL || bucket->ip[0] == '\0') {
        perror("Invalid table or bucket");
        return -1;
    }
    int hash = hash_ip(bucket->ip);
    if (hash < 0) {
        perror("Invalid hash");
        return -1;
    }
    bucket->next = table->buckets[hash];
    table->buckets[hash] = bucket;
    return 0;
}

bucket_t *table_get(table_t *table, const char ip[IP_LEN]) {
    if (table == NULL || ip == NULL) {
        perror("Invalid table or ip");
        return NULL;
    }
    int hash = hash_ip(ip);
    if (hash < 0 || hash >= TABLE_LEN) {
        perror("Invalid hash");
        return NULL;
    }
    bucket_t *current = table->buckets[hash];
    while (current != NULL) {
        if (strcmp(current->ip, ip) == 0) {
            return current;
        }
        current = current -> next;
    }
    return NULL;
}

int hash_ip(const char ip[IP_LEN]) {
    if (ip == NULL) {
        perror("Invalid ip");
        return -1;
    }
    int hash = 0;
    for (int i = 0; i < IP_LEN; i++) {
        hash += ip[i];
    }
    return hash % TABLE_LEN;
}

int table_to_file(table_t *table, const char out_file[MAX_PATH]) {
    if (table == NULL || out_file == NULL) {
        perror("Invalud table or out_file");
        return -1;
    }
    FILE *file = fopen(out_file,"wb");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    for (int i = 0; i < TABLE_LEN; i++) {
        bucket_t *current = table->buckets[i];
        while (current != NULL) {
            fwrite(current, sizeof(bucket_t), 1, file);
            current = current->next;
        }
    }
    fclose(file);
    return 0;
}

table_t *table_from_file(const char in_file[MAX_PATH]) {
    if (in_file == NULL) {
        perror("Invalid in_file");
        return NULL;
    }
    FILE *file = fopen(in_file, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        fclose(file);
        return NULL;
    }
    table_t *table = table_init();
    if (table == NULL) {
        perror("Failed to allocate memory for table");
        fclose(file);
        return NULL;
    }
    bucket_t *bucket = malloc(sizeof(bucket_t));
    if (bucket == NULL) {
        perror("Failed to allocate memory for bucket");
        fclose(file);
        return NULL;
    }
    while (fread(bucket, sizeof(bucket_t),1,file) > 0) {
        bucket_t *new_bucket = bucket_init(bucket->ip);
        new_bucket->requests = bucket->requests;
        table_add(table,new_bucket);
    }
    fclose(file);
    return table;
}
