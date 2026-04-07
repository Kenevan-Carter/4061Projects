#include "ml_thread.h"
#include <stdio.h>
#include <stdlib.h>

void* producer(void* arg) {
    int num_workers = *(int*)arg;
    /**
    * TODO: Producer thread logic.
    * * The producer is responsible for:
    * 1. Opening the image and label binary files.
    * 2. Reading image data and labels into InferenceTask structures.
    * 3. Safely adding tasks to the shared circular buffer (handling full buffer).
    * 4. Sending a termination signal to each worker thread.
    * * Please refer to the Lab Writeup for details on circular buffer management 
    * and thread synchronization using mutex/condition variables.
    */

    // --- YOUR CODE STARTS HERE ---


    // --- YOUR CODE ENDS HERE ---


    return NULL;
}