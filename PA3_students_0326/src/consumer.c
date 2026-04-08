#include "ml_thread.h"
#include <stdio.h>

void* consumer(void* arg) {
    /**
    * TODO: Consumer thread logic.
    * * Each consumer is responsible for:
    * 1. Safely retrieving a task from the shared circular buffer.
    * 2. Checking for the termination signal (is_last flag).
    * 3. Performing matrix-vector multiplication.
    * 4. Finding the predicted class.
    * 5. Storing the result in the global 'predictions' array.
    * * Refer to the Lab Writeup for synchronization details and the 
    * mathematical definition of the inference task.
    */

    // --- YOUR CODE STARTS HERE ---
    (void)arg;

    while (1) {
        InferenceTask task;

        // Take one task from the shared queue
        pthread_mutex_lock( &mutex);
        while (count == 0) {
            pthread_cond_wait(&not_empty,&mutex);
        }





        task = queue[head];
        head = (head + 1) %BUFFER_SIZE;
        count--;
        // Wake producer if it was waiting for space
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        // kill when termination task is run
        if (task.is_last) {
            break;
        }

        //matrix vector multiplication
        float output[NUM_CLASSES];

        for (int j = 0; j < NUM_CLASSES; j++) {
            output[j] = 0.0f;
            for (int k = 0; k < IMG_SIZE; k++) {
                output[j] += task.pixels[k] * weights[k][j];
            }
        }

        //find the class with the highest score.
        int best_class = 0;
        for (int j = 1; j < NUM_CLASSES; j++) {
            if (output[j] > output[best_class]) {
                best_class = j;
            }
        }

        //store the class prediction
        predictions[task.image_id] = best_class;
    }


    // --- YOUR CODE ENDS HERE ---
    return NULL;
}