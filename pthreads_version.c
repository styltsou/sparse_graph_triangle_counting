#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "coo2csc.h"
#include "mergesort.h"
#include "mmio.h"
#include "utils.h"

// Matrix data
int M, N, nz;
int *lower_coo_rows, *lower_coo_cols, *lower_coo_values;

// Store CSC data for the whole adjacency matrix
int *indices;
int *pointers;
int *values;

// Variables store the lower triangle of the masked matrix product in COO format
int *lower_res_rows, *lower_res_cols, *lower_res_values;
// Variables to store the whole resulting matrix in CSC format
int *res_indices, *res_pointers, *res_values;
// Number of non-zero elemnts in the resulting matrix
int res_nnz = 0;

// Struct used to pass arguments in the function executed in parallel
typedef struct {
  int tid;
  int starting_nz;
  int num_nz;
} ThreadArgs;

pthread_mutex_t mutex;

void *compute_sub_hadamard(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int result;

  for (int i = args->starting_nz; i < args->starting_nz + args->num_nz; ++i) {
    result = get_common_subarray_items_count(
        indices, pointers[lower_coo_rows[i]], pointers[lower_coo_rows[i] + 1],
        pointers[lower_coo_cols[i]], pointers[lower_coo_cols[i] + 1]);

    if (result != 0) {
      pthread_mutex_lock(&mutex);
      lower_res_rows[res_nnz] = lower_coo_rows[i];
      lower_res_cols[res_nnz] = lower_coo_cols[i];
      lower_res_values[res_nnz] = result;

      res_nnz++;

      pthread_mutex_unlock(&mutex);
    }
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  double t = clock();
  char *filepath;
  MM_typecode *matcode;
  int ret_code;
  int num_threads;

  parse_cli_args_parallel(argc, argv, &filepath, &num_threads);

  // Read mtx file
  ret_code = mm_read_mtx_crd(filepath, &M, &N, &nz, &lower_coo_rows,
                             &lower_coo_cols, &lower_coo_values, &matcode);

  if (ret_code) {
    fprintf(stderr, "Error while reading mm file.");
    exit(1);
  }

  // Convert to zero-based indexing for compatibility with C arrays
  for (int i = 0; i < nz; i++) {
    lower_coo_rows[i]--;
    lower_coo_cols[i]--;
  }

  int *full_rows = (int *)malloc(2 * nz * sizeof(int));
  int *full_cols = (int *)malloc(2 * nz * sizeof(int));
  int *full_values = (int *)malloc(2 * nz * sizeof(int));

  for (int i = 0; i < nz; i++) {
    full_rows[i] = lower_coo_rows[i];
    full_rows[nz + i] = lower_coo_cols[i];
    full_cols[i] = lower_coo_cols[i];
    full_cols[nz + i] = lower_coo_rows[i];
  }

  indices = (int *)malloc(2 * nz * sizeof(int));
  pointers = (int *)malloc((N + 1) * sizeof(int));
  // No need to allocate memory for vlaues array as it is not processed by the
  // coo2csc() routine

  coo2csc(indices, pointers, values, full_rows, full_cols, full_values, 2 * nz,
          N, 0, 1);

  for (int i = 0; i < N; i++) {
    mergeSort(indices, pointers[i], pointers[i + 1] - 1);
  }

  // Allocate memory to store the results for the matrix product
  lower_res_rows = (int *)malloc(nz * sizeof(int));
  lower_res_cols = (int *)malloc(nz * sizeof(int));
  lower_res_values = (int *)malloc(nz * sizeof(int));

  // Allocate memory for thread ids and arguments
  pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
  ThreadArgs *args = (ThreadArgs *)malloc(num_threads * sizeof(ThreadArgs));

  // Calculate the number of non-zeros to be processed by each thread
  int n_nz = nz / num_threads;
  int remainder_nz = nz % num_threads;
  int rc;

  pthread_mutex_init(&mutex, NULL);

  for (long i = 0; i < num_threads; i++) {
    // populate args struct
    args[i].tid = (int)i;
    args[i].num_nz = n_nz;
    args[i].starting_nz = i * args[i].num_nz;
    if (i == num_threads - 1) args[i].num_nz += remainder_nz;

    // Create threads
    rc = pthread_create(&threads[i], NULL, compute_sub_hadamard,
                        (void *)&args[i]);
  }

  // Join threads after finishing execution
  for (long t = 0; t < num_threads; t++) {
    rc = pthread_join(threads[t], NULL);
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

  pthread_mutex_destroy(&mutex);

  // Reallocate memory as needed
  lower_res_rows = (int *)realloc(lower_res_rows, res_nnz * sizeof(int));
  lower_res_cols = (int *)realloc(lower_res_cols, res_nnz * sizeof(int));
  lower_res_values = (int *)realloc(lower_res_values, res_nnz * sizeof(int));

  // Arrays to store the COO format of the product result (for the whole matrix)
  int *full_res_rows = (int *)malloc(2 * res_nnz * sizeof(int));
  int *full_res_cols = (int *)malloc(2 * res_nnz * sizeof(int));
  int *full_res_values = (int *)malloc(2 * res_nnz * sizeof(int));

  for (int i = 0; i < res_nnz; i++) {
    full_res_rows[i] = lower_res_rows[i];
    full_res_rows[res_nnz + i] = lower_res_cols[i];
    full_res_cols[i] = lower_res_cols[i];
    full_res_cols[res_nnz + i] = lower_res_rows[i];
    full_res_values[i] = lower_res_values[i];
    full_res_values[res_nnz + i] = lower_res_values[i];
  }

  // Variables to store the result in CSC format
  res_indices = (int *)malloc(2 * res_nnz * sizeof(int));
  res_pointers = (int *)malloc((N + 1) * sizeof(int));
  res_values = (int *)malloc(2 * res_nnz * sizeof(int));

  coo2csc(res_indices, res_pointers, res_values, full_res_rows, full_res_cols,
          full_res_values, 2 * res_nnz, N, 0, 0);

  // Sort the indices for every column to get the correct CSC format
  for (int i = 0; i < N; i++) {
    mergeSort_mirror(res_indices, res_values, res_pointers[i],
                     res_pointers[i + 1] - 1);
  }

  int *triangles_vector = (int *)calloc(N, sizeof(int));

  for (int i = 0; i < N; i++) {
    for (int j = res_pointers[i]; j < res_pointers[i + 1]; j++) {
      triangles_vector[i] += res_values[j];
    }

    triangles_vector[i] /= 2;
  }

  int triangle_count = 0;
  for (int i = 0; i < N; i++) {
    triangle_count += triangles_vector[i];
  }

  double total_time = (double)(clock() - t) / CLOCKS_PER_SEC;
  // printf("Triangle count : %d\n", triangle_count / 3);
  fprintf(stdout, "%lf\n", total_time);
}
