#include "proc_struct.h"
#include "proc_sort.h"

void swap_process(process* a, process* b) {
    process temp = *a;
    *a = *b;
    *b = temp;
}

int compare_pid(const process* a, const process* b) {
    return a->pid - b->pid;
}

int compare_ppid(const process* a, const process* b) {
    return a->ppid - b->ppid;
}

int compare_start(const process* a, const process* b) {
    return a->start - b->start;
}

int compare_total(const process* a, const process* b) {
    return a->total_time - b->total_time;
}

int compare_cpu(const process* a, const process* b) {
    return a->cpu_usage - b->cpu_usage;
}

int compare_mem(const process* a, const process* b) {
    return a->mem_usage - b->mem_usage;
}

void merge(process arr[], int l, int m, int r, int (*comparator)(const process*, const process*)) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    process L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (comparator(&L[i], &R[j]) <= 0) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}


void merge_sort(process arr[], int l, int r, int (*comparator)(const process*, const process*)) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort(arr, l, m, comparator);
        merge_sort(arr, m + 1, r, comparator);
        merge(arr, l, m, r, comparator);
    }
}