#ifndef MERGESORT_H
#define MERGESORT_H

void merge(int *arr, int low, int mid, int high);

void mergeSort(int *arr, int low, int high);

void merge_mirror(int *arr, int *m_arr, int low, int mid, int high);

void mergeSort_mirror(int *arr, int *m_arr, int low, int high);

#endif