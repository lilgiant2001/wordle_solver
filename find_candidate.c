#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "wordlist_helper.h"

// Struct for a frequency thread argument
typedef struct thread_freq_arg {
  // This field stores the word length
  int word_length;
  // This field stores the starting address of the file data
  char* file_data;
  // This field stores the number of words in the file data
  int file_num_words;
  // This filed stores the frequency table
  int** frequency;
} thread_freq_arg_t;

/**
 * The thread function to calculate the frequency of each letter on each location
 *
 * \param arg  The address of the sturct for this thread function
 * \returns    NULL
 */
void* thread_freq_fn(void* arg) {
  // Get arguments
  thread_freq_arg_t* threads_freq_args = (thread_freq_arg_t*)arg;

  int word_length = threads_freq_args->word_length;
  char* file_data = threads_freq_args->file_data;
  int file_num_words = threads_freq_args->file_num_words;
  int** frequency = threads_freq_args->frequency;

  // Traverse the given word list and calculate the frequency of each letter on each location
  for (int i = 0; i < file_num_words; i++) {
    // Update the frequency table based on the current word
    for (int j = 0; j < word_length; j++) {
      frequency[j][file_data[j] - 'a']++;
    }

    // Move to the next word
    file_data += word_length + 1;
  }

  // Return NULL
  return NULL;
}

// Struct for a candidate thread argument
typedef struct thread_cand_arg {
  // This field stores the word length
  int word_length;
  // This field stores the starting address of the file data
  char* file_data;
  // This field stores the number of words in the file data
  int file_num_words;
  // This filed stores the probability table
  double** probability;
  // This field stores the highest score
  double highest;
  // This field stores the word with the highest score
  char* highest_word;
} thread_cand_arg_t;

/**
 * The thread function to find the candidate word
 * It scores each word in the word list and finds the word with the higest score
 *
 * \param arg  The address of the sturct for this thread function
 * \returns    NULL
 */
void* thread_cand_fn(void* arg) {
  // Get arguments
  thread_cand_arg_t* threads_freq_args = (thread_cand_arg_t*)arg;

  int word_length = threads_freq_args->word_length;
  char* file_data = threads_freq_args->file_data;
  int file_num_words = threads_freq_args->file_num_words;
  double** probability = threads_freq_args->probability;

  // Traverse the given word list and score each word based on the probability table
  for (size_t i = 0; i < file_num_words; i++) {
    // Initalize the score of the current word
    double score = 0;

    // Calculate the score of the current word
    for (int j = 0; j < word_length; j++) {
      score += probability[j][file_data[j] - 'a'];
    }

    // Update the word with the highest score
    if (i == 0) {
      threads_freq_args->highest = score;
      strncpy(threads_freq_args->highest_word, file_data, word_length);
      threads_freq_args->highest_word[word_length] = '\0';
    } else if (score > threads_freq_args->highest) {
      threads_freq_args->highest = score;
      strncpy(threads_freq_args->highest_word, file_data, word_length);
      threads_freq_args->highest_word[word_length] = '\0';
    }

    // Move to the next word
    file_data += word_length + 1;
  }

  // Return NULL
  return NULL;
}

/**
 * The function finds a candidate word in the word list
 *
 * \param word_candidate  the address where the candidate word will be stored
 * \param word_length     the length of words in the word list
 * \param file_path       the path of the word list file
 * \param num_threads     the number of threads that will be used
 */
void find_candidate(char* word_candidate, int word_length, char* file_path, int num_threads) {
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

  // Initalize frequency threads and their arguments
  pthread_t threads_freq[num_threads];
  thread_freq_arg_t threads_freq_args[num_threads];

  // Create a three dimensional array for the frequency table of each frequency thread
  int*** frequency = malloc(sizeof(int**) * num_threads);
  for (int i = 0; i < num_threads; i++) {
    frequency[i] = malloc(sizeof(int*) * word_length);
    for (int j = 0; j < word_length; j++) {
      frequency[i][j] = malloc(sizeof(int) * NUM_LETTER);
      for (int k = 0; k < NUM_LETTER; k++) {
        frequency[i][j][k] = 0;
      }
    }
  }

  // Divide the file size by the number of threads
  int num_words = file_size / (word_length + 1);
  int file_num_words = num_words / num_threads;

  // Assign the quotient to each frequency thread argument
  for (size_t i = 0; i < num_threads; i++) {
    threads_freq_args[i].file_num_words = file_num_words;
  }

  // Distribute the remainder to frequency thread arguments
  for (size_t i = 0; i < num_words % num_threads; i++) {
    threads_freq_args[i].file_num_words++;
  }

  // Store the starting address of the file data in the first frequency thread argument
  threads_freq_args[0].file_data = file_data;

  // Store the starting address of each portion in each frequency thread argument
  for (size_t i = 1; i < num_threads; i++) {
    threads_freq_args[i].file_data = threads_freq_args[i - 1].file_data + threads_freq_args[i - 1].file_num_words * (word_length + 1);
  }

  // Store the word length and the frequency table in each frequency thread argument
  for (size_t i = 0; i < num_threads; i++) {
    threads_freq_args[i].word_length = word_length;
    threads_freq_args[i].frequency = frequency[i];
  }

  // Create frequency threads
  for (size_t i = 0; i < num_threads; i++) {
    if (pthread_create(&threads_freq[i], NULL, thread_freq_fn, &threads_freq_args[i])) {
      // Check an error for pthread_create
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Join the frequency threadsthreads_freq
  for (size_t i = 0; i < num_threads; i++) {
    // Check an error for pthread_join
    if (pthread_join(threads_freq[i], NULL)) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

  // Create two dimensional array for the frequency table for the whole word list
  int** frequency_total = malloc(sizeof(int*) * word_length);
  for (int i = 0; i < word_length; i++) {
    frequency_total[i] = malloc(sizeof(int) * NUM_LETTER);
    for (int j = 0; j < NUM_LETTER; j++) {
      frequency_total[i][j] = 0;
    }
  }

  // Combine the results fomr each frequency thread
  for (int i = 0; i < num_threads; i++) {
    for (int j = 0; j < word_length; j++) {
      for (int k = 0; k < NUM_LETTER; k++) {
        frequency_total[j][k] += frequency[i][j][k];
      }
    }
  }

  // Create two dimensional array for the probability table
  double** probability = malloc(sizeof(double*) * word_length);
  for (int i = 0; i < word_length; i++) {
    probability[i] = malloc(sizeof(double) * NUM_LETTER);
    for (int j = 0; j < NUM_LETTER; j++) {
      probability[i][j] = 0;
    }
  }

  // Update the probabiliy table based on the frequency table
  for (int i = 0; i < word_length; i++) {
    int total = 0;

    for (int j = 0; j < NUM_LETTER; j++) {
      total += frequency_total[i][j];
    }

    for (int j = 0; j < NUM_LETTER; j++) {
      probability[i][j] = (double)frequency_total[i][j] / total;
    }
  }

  // Initalize candidate threads and their arguments
  pthread_t threads_cand[num_threads];
  thread_cand_arg_t threads_cand_args[num_threads];

  // Store the number of words in the word list for each thread
  for (size_t i = 0; i < num_threads; i++) {
    threads_cand_args[i].file_num_words = threads_freq_args[i].file_num_words;
  }

  // Store the starting address of the file data in the first thread argument
  threads_cand_args[0].file_data = file_data;

  // Store the starting address of each portion in each thread argument
  for (size_t i = 1; i < num_threads; i++) {
    threads_cand_args[i].file_data = threads_cand_args[i - 1].file_data + threads_cand_args[i - 1].file_num_words * (word_length + 1);
  }

  // Store the word length and the probability table in each frequency thread argument and initalize the highest score and the word with the highest score
  for (size_t i = 0; i < num_threads; i++) {
    threads_cand_args[i].word_length = word_length;
    threads_cand_args[i].probability = probability;
    threads_cand_args[i].highest_word = malloc(sizeof(char) + (word_length + 1));
    threads_cand_args[i].highest = 0;
  }

  // Create candidate threads
  for (size_t i = 0; i < num_threads; i++) {
    if (pthread_create(&threads_cand[i], NULL, thread_cand_fn, &threads_cand_args[i])) {
      // Check an error for pthread_create
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Join the candidate threads
  for (size_t i = 0; i < num_threads; i++) {
    // Check an error for pthread_join
    if (pthread_join(threads_cand[i], NULL)) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

  // Find the highest score and the word with the highest score from each thread
  int highest = 0;

  for (size_t i = 0; i < num_threads; i++) {
    if (threads_cand_args[i].highest > highest) {
      highest = threads_cand_args[i].highest;
      strncpy(word_candidate, threads_cand_args[i].highest_word, word_length);
      word_candidate[word_length] = '\0';
    }
  }

  // Free all the malloced memory
  for (size_t i = 0; i < num_threads; i++) {
    free(threads_cand_args[i].highest_word);
  }

  for (int i = 0; i < word_length; i++) {
    free(probability[i]);
  }
  free(probability);

  for (int i = 0; i < word_length; i++) {
    free(frequency_total[i]);
  }
  free(frequency_total);

  for (int i = 0; i < num_threads; i++) {
    for (int j = 0; j < word_length; j++) {
      free(frequency[i][j]);
    }
    free(frequency[i]);
  }
  free(frequency);

  close(fd);
}

/**
 * The function finds a candidate word in the word list
 *
 * \param word_candidate  the address where the candidate word will be stored
 * \param word_length     the length of words in the word list
 * \param file_path       the path of the word list file
 * \param num_threads     the number of threads that will be used
 */
void find_candidate_random(char* word_candidate, int word_length, char* file_path) {
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

  // Divide the file size by the number of threads
  int num_words = file_size / (word_length + 1);

  srand(time(NULL));
  int random_word_index = rand() % num_words;

  strncpy(word_candidate, file_data + random_word_index * (word_length + 1), word_length);

  word_candidate[word_length] = '\0';

  close(fd);
}