#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "coo2csc.h"
#include "mergesort.h"
#include "mmio.h"
#include "utils.h"

// Number of triangles for each graph
// belgium_osm: 2.420
// com-Youtube: 3.056.386
// mycielskian13 : 0
// dblp-2010 : 1.676.652

int main(int argc, char *argv[]) {
  clock_t t = clock();
  char *file_path;
  MM_typecode *matcode;
  int M, N, nnz;
  int *lower_coo_rows, *lower_coo_cols, *lower_coo_values;
  int ret_code;

  /*
    Variables to store the result of the masked matrix product in COO format
    They contain only the lower triangle
  */
  int *lower_res_rows, *lower_res_cols, *lower_res_values;
  int res_nnz;

  parse_cli_args(argc, argv, &file_path);

  // Parse specified MM file and store its data
  ret_code = mm_read_mtx_crd(file_path, &M, &N, &nnz, &lower_coo_rows,
                             &lower_coo_cols, &lower_coo_values, &matcode);

  if (ret_code) {
    fprintf(stderr, "Error while reading MM file.");
    exit(1);
  }

  // Convert to zero-based indexing for compatibility with C arrays
  for (int i = 0; i < nnz; i++) {
    lower_coo_rows[i]--;
    lower_coo_cols[i]--;
  }

  // Allocate memory and create COO coordinates for the whole matrix
  int *full_rows = (int *)malloc(2 * nnz * sizeof(int));
  int *full_cols = (int *)malloc(2 * nnz * sizeof(int));
  int *full_values = (int *)malloc(2 * nnz * sizeof(int));

  for (int i = 0; i < nnz; i++) {
    full_rows[i] = lower_coo_rows[i];
    full_rows[nnz + i] = lower_coo_cols[i];
    full_cols[i] = lower_coo_cols[i];
    full_cols[nnz + i] = lower_coo_rows[i];
  }

  // Convert COO format to CSC
  int *indices = (int *)malloc(2 * nnz * sizeof(int));
  int *pointers = (int *)malloc((N + 1) * sizeof(int));
  int *values = (int *)malloc(2 * nnz * sizeof(int));

  coo2csc(indices, pointers, values, full_rows, full_cols, full_values, 2 * nnz,
          N, 0, 1);

  free(full_rows);
  free(full_cols);

  // Sort the indices for every column to get the correct CSC format
  for (int i = 0; i < N; i++)
    mergeSort(indices, pointers[i], pointers[i + 1] - 1);

  // Store the lower triangle of the masked matrix product in COO format
  // The resulting matrix contains at most, nnz non-zeros
  lower_res_rows = (int *)malloc(nnz * sizeof(int));
  lower_res_cols = (int *)malloc(nnz * sizeof(int));
  lower_res_values = (int *)malloc(nnz * sizeof(int));

  int result;
  // Current number of non zeros of the resulting matrix
  res_nnz = 0;
  // Compute the product for every non-zero element in the adjacency matrix
  for (int i = 0, j = 0; i < nnz; ++i) {
    // nz indices of row[i] span from idx=pointers[row[i]] to
    // idx=pointers[row[i] + 1] nz indices of col[i] span from
    // idx=pointers[col[i]] to idx=pointers[col[i] + 1]
    result = get_common_subarray_items_count(
        indices, pointers[lower_coo_rows[i]], pointers[lower_coo_rows[i] + 1],
        pointers[lower_coo_cols[i]], pointers[lower_coo_cols[i] + 1]);

    if (result != 0) {
      lower_res_rows[res_nnz] = lower_coo_rows[i];
      lower_res_cols[res_nnz] = lower_coo_cols[i];
      lower_res_values[res_nnz] = result;

      (res_nnz)++;
    }
  }

  // Reallocate memory as needed
  lower_res_rows = realloc(lower_res_rows, res_nnz * sizeof(int));
  lower_res_cols = realloc(lower_res_cols, res_nnz * sizeof(int));
  lower_res_values = realloc(lower_res_values, res_nnz * sizeof(int));

  free(indices);
  free(pointers);

  // Get the COO format for the whole resulting matrix
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

  // Arrays to store the full resulting matrix in CSC format
  int *res_indices = (int *)malloc(2 * res_nnz * sizeof(int));
  int *res_pointers = (int *)malloc((N + 1) * sizeof(int));
  int *res_values = (int *)malloc(2 * nnz * sizeof(int));

  coo2csc(res_indices, res_pointers, res_values, full_res_rows, full_res_cols,
          full_res_values, 2 * res_nnz, N, 0, 0);

  // Sort the indices and mirror the postion changes in the values array
  for (int i = 0; i < N; i++) {
    mergeSort_mirror(res_indices, res_values, res_pointers[i],
                     res_pointers[i + 1] - 1);
  }

  // Array that holds the number of triangles for every node
  int *triangles_vector = (int *)calloc(N, sizeof(int));

  // Compute the matrix - vector product
  for (int i = 0; i < N; i++) {
    for (int j = res_pointers[i]; j < res_pointers[i + 1]; j++) {
      triangles_vector[i] += res_values[j];
    }

    triangles_vector[i] /= 2;
  }

  // Get the number of triangles for the whole graph
  // The result must be divided by 3
  int triangle_count = 0;
  for (int i = 0; i < N; i++) {
    triangle_count += triangles_vector[i];
  }

  double total_time = (double)(clock() - t) / CLOCKS_PER_SEC;
  printf("Triangle count : %d\n", triangle_count / 3);
  fprintf(stdout, "Total execution time: %lf\n", total_time);

  return 0;
}