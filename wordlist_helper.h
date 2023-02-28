#pragma once

// Number of letters
#define NUM_LETTER 26
// Page size
#define PAGE_SIZE 0x1000
// Round up operation for mmap
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

/**
 * The function finds a candidate word in the word list
 *
 * \param word_candidate  the address where the candidate word will be stored
 * \param word_length     the length of words in the word list
 * \param file_path       the path of the word list file
 * \param num_threads     the number of threads that will be used
 */
void find_candidate(char* word_candidate, int word_length, char* file_path, int num_threads);

/**
 * The function checkes whether the word is in the word list
 *
 * \param word            the word to be checked
 * \param word_length     the length of words in the word list
 * \param file_path       the path of the word list file
 * \param num_threads     the number of threads that will be used
 *
 * \return 0 if the word is contained in the word list
 *         1 otherwise
 */
int check_validity(char* word, int word_length, char* file_path, int num_threads);

void filter_wordlist(char* word, char* word_candidate, int word_length, char* file_path, int num_threads);

void find_candidate_random(char* word_candidate, int word_length, char* file_path);