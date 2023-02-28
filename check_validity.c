#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "wordlist_helper.h"

// Struct for a search thread argument
typedef struct thread_search_arg {
  // This field stores the word length
  int word_length;
  // This field stores the starting address of the file data
  char* file_data;
  // This field stores the number of words in the file data
  int file_num_words;
  // This field stores the word
  char* word;
  // This field stores the value that indicates that whether the word is in the word list
  int contain;
} thread_search_arg_t;

/**
 * The thread function to calculate the search of each letter on each location
 *
 * \param arg  The address of the sturct for this thread function
 * \returns    NULL
 */
void* thread_search_fn(void* arg) {
  // Get arguments
  thread_search_arg_t* threads_search_args = (thread_search_arg_t*)arg;

  int word_length = threads_search_args->word_length;
  char* file_data = threads_search_args->file_data;
  int file_num_words = threads_search_args->file_num_words;
  char* word = threads_search_args->word;

  // Traverse the given word list and calculate the search of each letter on each location
  for (int i = 0; i < file_num_words; i++) {
    if (strncmp(file_data, word, word_length) == 0) {
      threads_search_args->contain = 1;
      return NULL;
    }

    // Move to the next word
    file_data += word_length + 1;
  }

  // Return NULL
  return NULL;
}

/**
 * The function checkes whether the word is in the word list
 *
 * \param word            the word to be checked
 * \param word_length     the length of words in the word list
 * \param file_path       the path of the word list file
 * \param num_threads     the number of threads that will be used
 *
 * \return 1 if the word is contained in the word list
 *         0 otherwise
 */
int check_validity(char* word, int word_length, char* file_path, int num_threads) {
  // Open the word list
  int fd = open(file_path, O_RDONLY);
  if (fd == -1) {
    // Check an error for open
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  // Get the word list size
  off_t file_size = lseek(fd, 0, SEEK_END);
  if (file_size == -1) {
    // Check an error for lseek
    perror("lseek failed");
    exit(EXIT_FAILURE);
  }

  // Go back to the start of the file
  if (lseek(fd, 0, SEEK_SET)) {
    // Check an error for lseek
    perror("lseek failed");
    exit(EXIT_FAILURE);
  }

  // Load the file with mmap
  char* file_data = mmap(NULL, ROUND_UP(file_size, PAGE_SIZE), PROT_READ, MAP_PRIVATE, fd, 0);
  if (file_data == MAP_FAILED) {
    // Check an error for mmap
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  // Initalize search threads and their arguments
  pthread_t threads_search[num_threads];
  thread_search_arg_t threads_search_args[num_threads];

  // Divide the file size by the number of threads
  int num_words = file_size / (word_length + 1);
  int file_num_words = num_words / num_threads;

  // Assign the quotient to each search thread argument
  for (size_t i = 0; i < num_threads; i++) {
    threads_search_args[i].file_num_words = file_num_words;
  }

  // Distribute the remainder to search thread arguments
  for (size_t i = 0; i < num_words % num_threads; i++) {
    threads_search_args[i].file_num_words++;
  }

  // Store the starting address of the file data in the first search thread argument
  threads_search_args[0].file_data = file_data;

  // Store the starting address of each portion in each search thread argument
  for (size_t i = 1; i < num_threads; i++) {
    threads_search_args[i].file_data = threads_search_args[i - 1].file_data + threads_search_args[i - 1].file_num_words * (word_length + 1);
  }

  // Store the word length in each search thread argument
  for (size_t i = 0; i < num_threads; i++) {
    threads_search_args[i].word_length = word_length;
    threads_search_args[i].word = word;
    threads_search_args[i].contain = 0;
  }

  // Create search threads
  for (size_t i = 0; i < num_threads; i++) {
    if (pthread_create(&threads_search[i], NULL, thread_search_fn, &threads_search_args[i])) {
      // Check an error for pthread_create
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Join the search threads
  for (size_t i = 0; i < num_threads; i++) {
    // Check an error for pthread_join
    if (pthread_join(threads_search[i], NULL)) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

  // Combine the results from all the threads
  int total_contain = 0;

  for (size_t i = 0; i < num_threads; i++) {
    total_contain += threads_search_args[i].contain;
  }

  if (total_contain) {
    // If the word is contained in the word list, return 1
    return 1;
  } else {
    // Otherwise, return 0
    return 0;
  }

  close(fd);
}