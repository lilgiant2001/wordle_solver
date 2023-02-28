#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "data_generator.h"
#include "ui.h"
#include "wordlist_helper.h"

#define NUM_TRIALS 6

#define NUM_THREADS 8

#define MAX_NUM_LETTER 10
#define MIN_NUM_LETTER 2

void greeting(char* saying_greeting);
void get_word_length(char* question_word_length, int* word_length);
void get_choice(char* question_algorithm, int* choice);
void get_word(char* question_word, int word_length, char* word, char* data_file_name);
void get_algorithm(char* question_algorithm, int* algorithm);

int main(int argc, char** argv) {
  char* saying_greeting = "----------------------------------\n|  Welcome to our wordle solver!  |\n----------------------------------\n";
  char* question_word_length = "What is your word length?";
  char* question_choice = "Do you want to play a game?\n1. Yes\n2. No";
  char* ask_word = "Type your word!";
  char* question_word = "What is your word?";
  char* question_algorithm = "Which algorithm do you want to use?\n1. Random Candidate\n2. Statistically Optimal Candidate";

  greeting(saying_greeting);

  int word_length;

  get_word_length(question_word_length, &word_length);

  char* data_file_name = data_file_name_generator(word_length);

  if (!data_file_exists(data_file_name)) {
    data_file_generator(data_file_name, word_length);
  }

  char* temp_file_name = "temp";

  copy_file(temp_file_name, data_file_name);

  int choice;

  get_choice(question_choice, &choice);

  char word[word_length + 1];

  switch (choice) {
    case 1:
      find_candidate_random(word, word_length, data_file_name);
      for (int i = 0; i < NUM_TRIALS; i++) {
        char input[word_length + 1];

        get_word(ask_word, word_length, input, data_file_name);

        print_hint(word, input, word_length);

        if (strncmp(word, input, word_length) == 0) {
          printf("Good Job!\n");
          break;
        }
      }

      break;
    case 2:
      get_word(question_word, word_length, word, data_file_name);
      break;
  }

  int algorithm;

  get_algorithm(question_algorithm, &algorithm);

  int count = 0;

  while (1) {
    char word_candidate[word_length + 1];

    switch (algorithm) {
      case 1:
        find_candidate_random(word_candidate, word_length, temp_file_name);
        break;

      case 2:
        find_candidate(word_candidate, word_length, temp_file_name, NUM_THREADS);
        break;
    }

    print_hint(word, word_candidate, word_length);

    if (strncmp(word, word_candidate, word_length) == 0) {
      printf("Done.\n");
      break;
    }

    filter_wordlist(word, word_candidate, word_length, temp_file_name, NUM_THREADS);

    count++;
  }

  free(data_file_name);

  return 0;
}

void greeting(char* saying_greeting) {
  printf("%s\n", saying_greeting);
}

void get_word_length(char* question_word_length, int* word_length) {
  printf("%s\n", question_word_length);

  *word_length = 0;

  while (1) {
    char* line = NULL;
    size_t line_size = 0;
    int num_line = getline(&line, &line_size, stdin);

    if (num_line == -1) {
      printf("\nPlease retype your word length.\n");
      continue;
    }

    if (strlen(line) <= 1) {
      printf("\nThe word length is invalid. The valid word length is between %d and %d inclusive.\n", MIN_NUM_LETTER, MAX_NUM_LETTER);
      continue;
    }

    *word_length = atoi(line);

    if (*word_length < MIN_NUM_LETTER || *word_length > MAX_NUM_LETTER) {
      printf("\nThe word length is invalid. The valid word lengths is between %d and %d inclusive.\n", MIN_NUM_LETTER, MAX_NUM_LETTER);
      continue;
    }

    free(line);

    printf("\n");

    return;
  }
}

void get_choice(char* question_choice, int* choice) {
  printf("%s\n", question_choice);

  *choice = 0;

  while (1) {
    char* line = NULL;
    size_t line_size = 0;
    int num_line = getline(&line, &line_size, stdin);

    if (num_line == -1) {
      printf("\nPlease retype your choice.\n");
      continue;
    }

    if (strlen(line) != 2) {
      printf("\nPlease type a valid choice.\n");
      continue;
    }

    *choice = atoi(line);

    if (*choice != 1 && *choice != 2) {
      printf("\nPlease type a valid choice.\n");
      continue;
    }

    free(line);

    printf("\n");

    return;
  }
}

void get_word(char* question_word, int word_length, char* word, char* data_file_name) {
  printf("%s\n", question_word);

  while (1) {
    char* line = NULL;
    size_t line_size = 0;
    int num_line = getline(&line, &line_size, stdin);

    if (num_line == -1) {
      printf("\nPlease retype your word.\n");
      free(line);
      continue;
    }

    if (strlen(line) != word_length + 1) {
      printf("\nPlease type a word with the valid length.\n");
      free(line);
      continue;
    }

    strncpy(word, line, word_length);

    free(line);

    word[word_length] = '\0';

    if (!check_validity(word, word_length, data_file_name, 8)) {
      printf("\nPlease type a valid word.\n");
      continue;
    }

    printf("\n");

    return;
  }
}

void get_algorithm(char* question_algorithm, int* algorithm) {
  printf("%s\n", question_algorithm);

  *algorithm = 0;

  while (1) {
    char* line = NULL;
    size_t line_size = 0;
    int num_line = getline(&line, &line_size, stdin);

    if (num_line == -1) {
      printf("\nPlease retype your algorithm choice.\n");
      continue;
    }

    if (strlen(line) != 2) {
      printf("\nPlease type a valid choice.\n");
      continue;
    }

    *algorithm = atoi(line);

    if (*algorithm != 1 && *algorithm != 2) {
      printf("\nPlease type a valid choice.\n");
      continue;
    }

    free(line);

    printf("\n");

    return;
  }
}