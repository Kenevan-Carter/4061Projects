#include "ml_thread.h"
#include <stdio.h>
#include <stdlib.h>

void *producer(void *arg) {
  int num_workers = *(int *)arg;
  /**
   * TODO: Producer thread logic.
   * * The producer is responsible for:
   * 1. Opening the image and label binary files.
   * 2. Reading image data and labels into InferenceTask structures.
   * 3. Safely adding tasks to the shared circular buffer (handling full
   * buffer).
   * 4. Sending a termination signal to each worker thread.
   * * Please refer to the Lab Writeup for details on circular buffer management
   * and thread synchronization using mutex/condition variables.
   */

  // --- YOUR CODE STARTS HERE ---

  FILE *image_fd = fopen("data/images.bin", "rb");
  if (image_fd == NULL) {
    perror("Failed to open images.bin");
    return NULL;
  }
  FILE *label_fd = fopen("data/labels.bin", "rb");
  if (label_fd == NULL) {
    perror("Failed to open labels.bin");
    fclose(image_fd);
    return NULL;
  }
  for (int i = 0; i < TOTAL_IMAGES; i++) {
    InferenceTask task;
    task.image_id = i;
    task.is_last = 0;
    if (fread(task.pixels, sizeof(float), IMG_SIZE, image_fd) != IMG_SIZE) {
      perror("Failed to read image pixels");
      fclose(image_fd);
      fclose(label_fd);
      return NULL;
    }
    if (fread(&task.label, sizeof(int), 1, label_fd) != 1) {
      perror("Failed to read label");
      fclose(image_fd);
      fclose(label_fd);
      return NULL;
    }
    // One thread at a time can add to the buffer
    pthread_mutex_lock(&mutex);
    // If the buffer is full, wait for the condition variable to be signaled
    while (count == BUFFER_SIZE) {
        pthread_cond_wait(&not_full, &mutex);
    }
    // Add the task to the buffer
    queue[tail] = task;
    tail = (tail + 1) % BUFFER_SIZE;
    count++;
    // Signal the condition variable that the buffer is not empty
    pthread_cond_signal(&not_empty);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex);
  }
  fclose(image_fd);
  fclose(label_fd);
  for (int i = 0; i < num_workers; i++) {
    InferenceTask task = {0};
    task.is_last = 1;
    pthread_mutex_lock(&mutex);
    while (count == BUFFER_SIZE) {
        pthread_cond_wait(&not_full, &mutex);
    }
    queue[tail] = task;
    tail = (tail + 1) % BUFFER_SIZE;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
  }
  // --- YOUR CODE ENDS HERE ---
  return NULL;
}
