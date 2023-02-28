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

  int** hint_table;
  int* contains;
  char** filtered_list;
  int num_filtered_list;
} thread_search_arg_t;

/**
 * The thread function to calculate the search of each letter on each location
 *
 * \param arg  The address of the sturct for this thread function
 * \returns    NULL
 */
void* thread_filter_fn(void* arg) {
  // Get arguments
  thread_search_arg_t* threads_search_args = (thread_search_arg_t*)arg;

  int word_length = threads_search_args->word_length;
  char* file_data = threads_search_args->file_data;
  int file_num_words = threads_search_args->file_num_words;
  int** hint_table = threads_search_args->hint_table;
  int* contains = threads_search_args->contains;

  // Traverse the given word list and calculate the search of each letter on each location
  for (int i = 0; i < file_num_words; i++) {
    int valid = 1;

    for (int j = 0; j < NUM_LETTER; j++) {
      if (contains[j]) {
        int contain = 0;

        for (int k = 0; k < word_length; k++) {
          if (file_data[k] == j + 'a') {
            contain = 1;
            break;
          }
        }

        if (!contain) {
          valid = 0;
        }
      }
    }

    for (int j = 0; j < word_length; j++) {
      if (!hint_table[j][file_data[j] - 'a']) {
        valid = 0;
        break;
      }
    }

    if (valid) {
      if (threads_search_args->num_filtered_list == 0) {
        threads_search_args->filtered_list = malloc(sizeof(char*));
      } else {
        threads_search_args->filtered_list = realloc(threads_search_args->filtered_list, sizeof(char*) * (threads_search_args->num_filtered_list + 1));
      }

      threads_search_args->filtered_list[threads_search_args->num_filtered_list] = malloc(sizeof(char) * (word_length + 1));
      strncpy(threads_search_args->filtered_list[threads_search_args->num_filtered_list], file_data, word_length);
      threads_search_args->filtered_list[threads_search_args->num_filtered_list][word_length] = '\0';
      threads_search_args->num_filtered_list++;
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
void filter_wordlist(char* word, char* word_candidate, int word_length, char* file_path, int num_threads) {
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

  int** hint_table = malloc(sizeof(int*) * word_length);
  for (int i = 0; i < word_length; i++) {
    hint_table[i] = malloc(sizeof(int) * NUM_LETTER);
    for (int j = 0; j < NUM_LETTER; j++) {
      hint_table[i][j] = 1;
    }
  }

  int* contains = malloc(sizeof(int) * NUM_LETTER);
  for (int i = 0; i < NUM_LETTER; i++) {
    contains[i] = 0;
  }

  int* matches = malloc(sizeof(int) * word_length);
  for (int i = 0; i < word_length; i++) {
    matches[i] = 0;
  }

  for (int i = 0; i < word_length; i++) {
    if (word_candidate[i] == word[i]) {
      matches[i] = 1;
      for (int j = 0; j < NUM_LETTER; j++) {
        if (j != word_candidate[i] - 'a') {
          hint_table[i][j] = 0;
        }
      }
    }
  }

  for (int i = 0; i < word_length; i++) {
    int match = 0;
    if (!matches[i]) {
      for (int j = 0; j < word_length; j++) {
        if (!matches[j]) {
          if (word_candidate[i] == word[j]) {
            match = 1;
            contains[word_candidate[i] - 'a'] = 1;
            hint_table[i][word_candidate[i] - 'a'] = 0;
          }
        }
      }

      if (!match) {
        for (int j = 0; j < word_length; j++) {
          if (!matches[j]) {
            hint_table[j][word_candidate[i] - 'a'] = 0;
          }
        }
      }
    }
  }

  free(matches);

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
    threads_search_args[i].contains = contains;
    threads_search_args[i].hint_table = hint_table;
    threads_search_args[i].filtered_list = NULL;
    threads_search_args[i].num_filtered_list = 0;
  }

  // Create search threads
  for (size_t i = 0; i < num_threads; i++) {
    if (pthread_create(&threads_search[i], NULL, thread_filter_fn, &threads_search_args[i])) {
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

  FILE* wordlist = fopen(file_path, "w");
  if (wordlist == NULL) {
    perror("fopen failed");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < num_threads; i++) {
    for (int j = 0; j < threads_search_args[i].num_filtered_list; j++) {
      fprintf(wordlist, "%s\n", threads_search_args[i].filtered_list[j]);
    }
  }

  fclose(wordlist);

  for (int i = 0; i < word_length; i++) {
    free(hint_table[i]);
  }
  free(hint_table);
  free(contains);

  for (size_t i = 0; i < num_threads; i++) {
    for (int j = 0; j < threads_search_args[i].num_filtered_list; j++) {
      free(threads_search_args[i].filtered_list[j]);
    }

    if (threads_search_args[i].filtered_list != NULL) {
      free(threads_search_args[i].filtered_list);
    }
  }

  close(fd);
}