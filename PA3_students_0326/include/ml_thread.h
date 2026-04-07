#ifndef ML_THREAD_H
#define ML_THREAD_H

#include <pthread.h>

#define IMG_SIZE 784      // 28x28
#define NUM_CLASSES 10    // 0-9 digits
#define TOTAL_IMAGES 10000 // Images to process

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 10000  
#endif
// #define BUFFER_SIZE 10    // Bounded buffer capacity


// Task Structure
typedef struct {
    int image_id;
    float pixels[IMG_SIZE];
    int label;
    int is_last; // Poison Pill flag
} InferenceTask;

// Shared resources (extern declarations)
extern InferenceTask queue[BUFFER_SIZE];
extern int head, tail, count;
extern pthread_mutex_t mutex;
extern pthread_cond_t not_full;
extern pthread_cond_t not_empty;

extern float weights[IMG_SIZE][NUM_CLASSES];
extern int predictions[TOTAL_IMAGES];
extern int ground_truth[TOTAL_IMAGES];

// Function Prototypes
void* producer(void* arg);
void* consumer(void* arg);

#endif