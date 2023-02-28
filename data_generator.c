#include "data_generator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char* data_file_name_generator(int word_length) {
  int num_digit_word_length = 0;

  int temp_word_length = word_length;

  do {
    num_digit_word_length++;

    temp_word_length /= 10;
  } while (temp_word_length != 0);

  char* file_name = malloc(sizeof(char) * (4 + num_digit_word_length + 1));

  file_name[0] = 'w';
  file_name[1] = 'o';
  file_name[2] = 'r';
  file_name[3] = 'd';

  temp_word_length = word_length;

  for (int i = num_digit_word_length - 1; i >= 0; i--) {
    file_name[4 + i] = temp_word_length % 10 + '0';
    temp_word_length /= 10;
  }

  file_name[4 + num_digit_word_length] = '\0';

  return file_name;
}

int data_file_exists(char* file_name) {
  if (access(file_name, F_OK) == 0) {
    return 1;
  } else {
    return 0;
  }
}

int check_english(char* word, size_t word_len) {
  for (size_t i = 0; i < word_len; i++) {
    if (word[i] >= 'A' || word[i] <= 'Z') {
      word[i] = tolower(word[i]);
    }

    if (!(word[i] >= 'a' && word[i] <= 'z')) {
      return 0;
    }
  }

  return 1;
}

void data_file_generator(char* file_name, int word_length) {
  FILE* fr = fopen("/usr/share/dict/american-english", "r");
  FILE* fw = fopen(file_name, "w");

  if (fr == NULL) {
    perror("Failed to fopen");
    exit(EXIT_FAILURE);
  }

  if (fw == NULL) {
    perror("Failed to fopen");
    exit(EXIT_FAILURE);
  }

  char* line = NULL;
  size_t len = 0;
  size_t read_len = 0;

  while ((read_len = getline(&line, &len, fr)) != -1) {
    size_t word_len = read_len - 1;

    if (word_len == word_length && check_english(line, word_len)) {
      fwrite(line, sizeof(char), read_len, fw);
    }
  }

  fclose(fr);

  fclose(fw);

  free(line);
}

void copy_file(char* file_name_dest, char* file_name_source) {
  FILE* fd = fopen(file_name_dest, "w");
  FILE* fs = fopen(file_name_source, "r");
  if (fd == NULL) {
    perror("Failed to fopen");
    exit(EXIT_FAILURE);
  }

  if (fs == NULL) {
    perror("Failed to fopen");
    exit(EXIT_FAILURE);
  }

  char c;
  while ((c = fgetc(fs)) != EOF) {
    fputc(c, fd);
  }

  fclose(fd);
  fclose(fs);
}