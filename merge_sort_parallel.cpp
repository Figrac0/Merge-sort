#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

// Обычная сортировка слиянием
void merge(std::vector<int>& array, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> leftArray(n1);
    std::vector<int> rightArray(n2);

    for (int i = 0; i < n1; i++)
        leftArray[i] = array[left + i];
    for (int j = 0; j < n2; j++)
        rightArray[j] = array[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (leftArray[i] <= rightArray[j]) {
            array[k] = leftArray[i];
            i++;
        } else {
            array[k] = rightArray[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        array[k] = leftArray[i];
        i++;
        k++;
    }

    while (j < n2) {
        array[k] = rightArray[j];
        j++;
        k++;
    }
}

// Класс TaskBag 
class TaskBag {
public:
    TaskBag(size_t num_threads) : done(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        condition.wait(lock, [this]() { return !tasks.empty() || done; });
                        if (done && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ~TaskBag() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            done = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
    }

    template<class F>
    void add_task(F&& f) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool done;
};

// Параллельная сортировка с Taskbag
void parallelMergeSortTaskBag(std::vector<int>& array, int left, int right, TaskBag& taskbag) {
    const int threshold = 100000; 

    if (right - left <= threshold) {
        std::sort(array.begin() + left, array.begin() + right + 1);
    } else {
        if (left < right) {
            int mid = left + (right - left) / 2;

            std::future<void> leftTask = std::async(std::launch::async, [&array, left, mid, &taskbag]() {
                parallelMergeSortTaskBag(array, left, mid, taskbag);
            });
            std::future<void> rightTask = std::async(std::launch::async, [&array, mid, right, &taskbag]() {
                parallelMergeSortTaskBag(array, mid + 1, right, taskbag);
            });

            leftTask.get();
            rightTask.get();

            merge(array, left, mid, right);
        }
    }
}

bool isSorted(const std::vector<int>& array) {
    for (size_t i = 1; i < array.size(); ++i) {
        if (array[i - 1] > array[i]) {
            return false;
        }
    }
    return true;
}

template<typename Func>
double measurePerformance(Func func, std::vector<int>& array) {
    auto start = std::chrono::high_resolution_clock::now();
    func(array, 0, array.size() - 1);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count(); // Время в миллисекундах
}

int main() {
    const int size = 10000000; 
    std::vector<int> array(size);
    
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 10000; 
    }

    std::vector<int> arrayCopy = array; 
    double timeMergeSort = measurePerformance([](std::vector<int>& arr, int left, int right) {
        std::sort(arr.begin() + left, arr.begin() + right + 1);
    }, arrayCopy);
    
    TaskBag taskbag(std::thread::hardware_concurrency()); // Число потоков = числу ядер
    arrayCopy = array; 
    double timeTaskbagSort = measurePerformance([&taskbag](std::vector<int>& arr, int left, int right) {
        parallelMergeSortTaskBag(arr, left, right, taskbag);
    }, arrayCopy);

    
    std::cout << "Время обычной сортировки: " << timeMergeSort << " мс\n";
    std::cout << "Время сортировки с Taskbag: " << timeTaskbagSort << " мс\n";

   
    if (isSorted(arrayCopy)) {
        std::cout << "Массив успешно отсортирован с использованием Taskbag.\n";
    } else {
        std::cout << "Ошибка: массив не отсортирован!\n";
    }

    return 0;
}
