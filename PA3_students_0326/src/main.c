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