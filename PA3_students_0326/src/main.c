#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ml_thread.h"

InferenceTask queue[BUFFER_SIZE];
int head = 0, tail = 0, count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

float weights[IMG_SIZE][NUM_CLASSES];
int predictions[TOTAL_IMAGES];
int ground_truth[TOTAL_IMAGES];

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_workers>\n", argv[0]);
        return 1;
    }

    printf("Buffer Size: %d\n", BUFFER_SIZE);
    printf("Worker Threads: %s\n", argv[1]);


    /* * TODO:
     * 1. Initialize resources and load data files.
     * 2. Create producer and consumer threads.
     * 3. Implement thread joining and timing.
     * * Please refer to the Lab Writeup for detailed implementation requirements.
     */

    // --- YOUR CODE STARTS HERE ---
    FILE *fd = fopen("data/weights.bin", "rb");
    if (fd == NULL) {
        perror("Failed to open weights.bin");
        return 1;
    }

    FILE *ground_truth_fd = fopen("data/labels.bin", "rb");
    if (ground_truth_fd == NULL) {
        perror("Failed to open labels.bin");
        fclose(fd);
        return 1;
    }

    size_t n_read_ground_truth = fread(ground_truth, sizeof(int), TOTAL_IMAGES, ground_truth_fd);
    if (n_read_ground_truth != TOTAL_IMAGES) {
        perror("Failed to read correct number of ground truth");
        fclose(fd);
        fclose(ground_truth_fd);
        return 1;
    }
    fclose(ground_truth_fd);

    size_t n_read = fread(weights, sizeof(float), IMG_SIZE * NUM_CLASSES, fd);
    if (n_read != IMG_SIZE * NUM_CLASSES) {
        perror("Failed to read correct number of weights");
        fclose(fd);
        return 1;
    }
    fclose(fd);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int num_workers = atoi(argv[1]);
    if (num_workers <= 0) {
        printf("Number of workers must be greater than 0\n");
        return 1;
    }
    pthread_t producer_thread;
    pthread_t *workers = malloc((size_t)num_workers * sizeof(pthread_t));
    if (workers == NULL) {
        perror("Failed to allocate memory for workers");
        return 1;
    }
    pthread_create(&producer_thread, NULL, producer, &num_workers);
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&workers[i], NULL, consumer, NULL);
    }

    pthread_join(producer_thread, NULL);
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    // --- YOUR CODE ENDS HERE ---



    /* * =========================================================================
     * DO NOT MODIFY THE CODE BELOW THIS LINE
     * =========================================================================
     */

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Check Output (first 20)
    printf("----- Check your results with expected_20.txt -----\n");
    for (int i = 0; i < 20; i++) {
    printf("Image %d: Prediction = %d\n", i, predictions[i]);
    }

    // Verify Results
    int correct = 0;
    for (int i = 0; i < TOTAL_IMAGES; i++) {
        if (predictions[i] == ground_truth[i]) correct++;
    }

    printf("\nAccuracy: %.2f%% (%d/%d)\n", (float)correct / TOTAL_IMAGES * 100, correct, TOTAL_IMAGES);
    printf("Execution Time: %.4f seconds\n", elapsed);

    free(workers);
    return 0;
}
