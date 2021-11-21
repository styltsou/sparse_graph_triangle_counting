#ifndef UTILS_H
#define UTILS_H

void parse_cli_args(int argc, char *argv[], char *file_path[]);

void parse_cli_args_parallel(int argc, char *argv[], char *file_path[],
                             int *num_threads);

int get_common_subarray_items_count(int *array, int start_idx1, int end_idx1,
                                    int start_idx2, int end_idx2);

#endif