
#include "mergesort.h"

#include <stdio.h>
#include <stdlib.h>

void merge(int* arr, int low, int mid, int high) {
  int i, j, k;
  int n1 = mid - low + 1;
  int n2 = high - mid;

  int* left_arr = (int*)malloc(n1 * sizeof(int));
  int* right_arr = (int*)malloc(n2 * sizeof(int));

  for (i = 0; i < n1; i++) {
    left_arr[i] = arr[low + i];
  }

  for (j = 0; j < n2; j++) {
    right_arr[j] = arr[mid + 1 + j];
  }

  i = 0;
  j = 0;
  k = low;

  while (i < n1 && j < n2) {
    if (left_arr[i] <= right_arr[j]) {
      arr[k] = left_arr[i];
      i++;
    } else {
      arr[k] = right_arr[j];
      j++;
    }

    k++;
  }

  while (i < n1) {
    arr[k] = left_arr[i];
    i++;
    k++;
  }

  while (j < n2) {
    arr[k] = right_arr[j];
    j++;
    k++;
  }

  free(left_arr);
  free(right_arr);
}

void mergeSort(int* arr, int low, int high) {
  if (low < high) {
    int mid = low + (high - low) / 2;

    mergeSort(arr, low, mid);
    mergeSort(arr, mid + 1, high);

    merge(arr, low, mid, high);
  }
}

void merge_mirror(int* arr, int* m_arr, int low, int mid, int high) {
  int i, j, k;
  int n1 = mid - low + 1;
  int n2 = high - mid;

  int* left_arr = (int*)malloc(n1 * sizeof(int));
  int* left_m_arr = (int*)malloc(n1 * sizeof(int));

  int* right_arr = (int*)malloc(n2 * sizeof(int));
  int* right_m_arr = (int*)malloc(n2 * sizeof(int));

  for (i = 0; i < n1; i++) {
    left_arr[i] = arr[low + i];
    left_m_arr[i] = m_arr[low + i];
  }

  for (j = 0; j < n2; j++) {
    right_arr[j] = arr[mid + 1 + j];
    right_m_arr[j] = m_arr[mid + 1 + j];
  }

  i = 0;
  j = 0;
  k = low;

  while (i < n1 && j < n2) {
    if (left_arr[i] <= right_arr[j]) {
      arr[k] = left_arr[i];
      m_arr[k] = left_m_arr[i];
      i++;
    } else {
      arr[k] = right_arr[j];
      m_arr[k] = right_m_arr[j];
      j++;
    }

    k++;
  }

  while (i < n1) {
    arr[k] = left_arr[i];
    m_arr[k] = left_m_arr[i];
    i++;
    k++;
  }

  while (j < n2) {
    arr[k] = right_arr[j];
    m_arr[k] = right_m_arr[j];
    j++;
    k++;
  }

  free(left_arr);
  free(left_m_arr);
  free(right_arr);
  free(right_m_arr);
}

void mergeSort_mirror(int* arr, int* m_arr, int low, int high) {
  if (low < high) {
    int mid = low + (high - low) / 2;

    mergeSort_mirror(arr, m_arr, low, mid);
    mergeSort_mirror(arr, m_arr, mid + 1, high);

    merge_mirror(arr, m_arr, low, mid, high);
  }
}