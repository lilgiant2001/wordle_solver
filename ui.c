#include "ui.h"

#include <stdio.h>
#include <stdlib.h>

// belows are functions that use ANSI color code to change the color of our output
void green() {
  printf(GRN);
}

void yellow() {
  printf(YEL);
}

void reset() {
  printf(WHT);
}
/**
 * \param answer the answer key which user types in
 * \param guess our candidate to check against answer
 * \param word_length length of our answer/guess
 **/
void print_hint(char* answer, char* guess, int word_length) {
  char* hint = malloc(sizeof(char) * word_length);
  for (int i = 0; i < word_length; i++) {
    hint[i] = '-';
  }

  int* answerFlags = malloc(sizeof(int) * word_length);

  for (int i = 0; i < word_length; i++) {
    answerFlags[i] = 0;
  }

  for (int i = 0; i < word_length; i++) {
    if (answer[i] == guess[i]) {
      hint[i] = 'G';
      answerFlags[i] = 1;
    }
  }
  for (int i = 0; i < word_length; i++) {
    if (hint[i] == '-') {  // means no exact match from first pass
      for (int j = 0; j < word_length; j++) {
        if (guess[i] == answer[j] && !answerFlags[j]) {
          // a match at another position and has not been used as clue
          hint[i] = 'Y';
          answerFlags[j] = 1;
          break;  // end this j-loop because we don't want multiple clues from the same letter
        }
      }
    }
  }
  // now we print out the candidate letter by letter
  for (int i = 0; i < word_length; i++) {
    if (hint[i] == 'G') {
      green();  // color the char green
      printf("%c", guess[i]);
    } else if (hint[i] == 'Y') {
      yellow();  // color the char green
      printf("%c", guess[i]);
    } else {
      reset();  // color the char white (reset the color)
      printf("%c", guess[i]);
    }
  }
  reset();
  printf("\n\n");

  free(answerFlags);
  free(hint);
}