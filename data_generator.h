#pragma once

char* data_file_name_generator(int word_length);

int data_file_exists(char* file_path);

void data_file_generator(char* file_path, int word_length);

void copy_file(char* file_name_dest, char* file_name_source);