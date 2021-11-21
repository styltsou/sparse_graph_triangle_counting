#include "utils.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Parse the command line arguments for the sequential implementation executable
void parse_cli_args(int argc, char *argv[], char *file_path[]) {
  char cwd[PATH_MAX];

  if (argc < 2) {
    fprintf(stderr, "Missing argument. You must specify a file path.\n");
    exit(1);
  }

  // Get current working directory
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "getcwd(): Error\n");
    exit(1);
  }

  // Remove '.' from the start of the relative path argument
  *argv[1] = *argv[1] + 1;
  // Absolute file path
  *file_path = strcat(cwd, argv[1]);
}

// Parse the command line arguments for the parallel implementation executables
void parse_cli_args_parallel(int argc, char *argv[], char *file_path[],
                             int *num_threads) {
  char cwd[PATH_MAX];

  if (argc < 2) {
    fprintf(stderr, "Missing argument. You must specify a file path.\n");
    exit(1);
  }

  // Get current working directory
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "getcwd(): Error\n");
    exit(1);
  }

  // Remove '.' from the start of the relative path argument
  *argv[1] = *argv[1] + 1;
  // Absolute file path
  *file_path = strcat(cwd, argv[1]);

  *num_threads = atoi(argv[2]);
}

// Find the number of common elemnts between two slices of the array
// The following algorithm works only on sorted arrays
int get_common_subarray_items_count(int *array, int start_idx1, int end_idx1,
                                    int start_idx2, int end_idx2) {
  int i = start_idx1;
  int j = start_idx2;
  int count = 0;

  while (i < end_idx1 && j < end_idx2) {
    if (array[i] < array[j])
      i++;
    else if (array[j] < array[i])
      j++;
    else {
      count++;
      i++;
      j++;
    }
  }

  return count;
}