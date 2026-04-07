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


    // --- YOUR CODE ENDS HERE ---


    return NULL;
}