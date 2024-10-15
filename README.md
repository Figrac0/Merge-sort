# Merge-sort
This repository contains two files that implement merge sort algorithms: a regular sort, a parallel sort using OpenMP, and a parallel sort with Taskbag.

Files<br>
1.merge_sort_openmp.cpp:<br>

Implements regular merge sort.<br>
Includes a parallel version of sort using OpenMP.<br>
Compares the performance of regular and parallel sorts based on execution time.<br>
2.merge_sort_taskbag.cpp:<br>

Implements parallel sort using Taskbag.<br>
Uses multithreading using standard threads and asynchronous tasks (std::async).<br>
Compares the execution time of sorting with Taskbag to regular sort.<br>
Run:<br>
Compile and run the programs with a compiler that supports OpenMP and C++11/14:<br>
1)g++ merge_sort_openmp.cpp -fopenmp -o merge_sort_openmp<br>
2)g++ merge_sort_taskbag.cpp -std=c++11 -o merge_sort_taskbag<br>
