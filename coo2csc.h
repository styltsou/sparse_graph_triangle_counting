#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void coo2csc(uint32_t *const row,           /*!< CSC row start indices */
             uint32_t *const col,           /*!< CSC column indices */
             int *values,                   /*<CSC values>*/
             uint32_t const *const row_coo, /*!< COO row indices */
             uint32_t const *const col_coo, /*!< COO column indices */
             int *values_coo,               /*!< COO values */
             uint32_t const nnz,            /*!< Number of nonzero elements */
             uint32_t const n,              /*!< Number of rows/columns */
             uint32_t const isOneBased,     /*!< Whether COO is 0- or 1-based */
             int isPattern /*Wether COO is a pattern matrix or not*/
);