#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

void merge(std::vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    std::vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// Параллельная сортировка слиянием (OpenMP)
void parallelMergeSort(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        if (right - left > 1000000) {
            #pragma omp parallel sections
            {
                #pragma omp section
                parallelMergeSort(arr, left, mid);
                #pragma omp section
                parallelMergeSort(arr, mid + 1, right);
            }
                // #pragma omp task firstprivate(arr,left,mid)
                //     parallelMergeSort(arr, left, mid);
                // #pragma omp task firstprivate(arr,right,mid)
                //     parallelMergeSort(arr, mid + 1, right);
                // #pragma omp taskbarrier
        } else {
            mergeSort(arr, left, right); 
        }

        merge(arr, left, mid, right);
    }
}

template <typename Func>
long long measureTime(Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

bool isSorted(const std::vector<int>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[i - 1]) return false;
    }
    return true;
}

int main() {
    int n = 10000000;  
    std::vector<int> arr(n), arrCopy1, arrCopy2;

    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
    }

    arrCopy1 = arr;
    arrCopy2 = arr;

    long long timeMergeSort = measureTime([&]() { mergeSort(arrCopy1, 0, arrCopy1.size() - 1); });
    std::cout << "Обычная сортировка слиянием: " << timeMergeSort << " ms" << std::endl;
    std::cout << "Отсортировано: " << (isSorted(arrCopy1) ? "Да" : "Нет") << std::endl;

    omp_set_nested(1);
    long long timeParallelMergeSort = measureTime([&]() {
            parallelMergeSort(arrCopy2, 0, arrCopy2.size() - 1); 
    });
    std::cout << "Параллельная сортировка слиянием (OpenMP): " << timeParallelMergeSort << " ms" << std::endl;
    std::cout << "Отсортировано: " << (isSorted(arrCopy2) ? "Да" : "Нет") << std::endl;

    return 0;
}
