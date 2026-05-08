#include "Mallocator.h"
#include "numa_Mallocator.h"
#include "print_time.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <numa.h>
#include <numaif.h>


using namespace std;

int main()
{
    // Define a vector with the custom allocator
    std::vector<int, Mallocator<int> > vec1(10);
    for (int i = 1; i <= 5; ++i) {
        vec1.push_back(i);
    }
    // Print the elements
    for (const auto& elem : vec1) {
        cout << elem << " ";
    }
    cout << endl;


    int N = 1024*1024*512;
    double out_vec=0, out_vec_a=0;
    struct timespec start, finish;
    char *str_log=(char *) malloc(500*sizeof(char));

    clock_gettime(CLOCK_REALTIME, &start);
    std::vector<double> vec(N); 
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector double alloc std time");
    print_time(str_log, start, finish);
    
    clock_gettime(CLOCK_REALTIME, &start);
    std::vector<int> vec_i(N); 
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector int alloc std time");
    print_time(str_log, start, finish);

    clock_gettime(CLOCK_REALTIME, &start);
    std::vector<double, Mallocator<double>> vec_a(N); 
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector double alloc allocator time");
    print_time(str_log, start, finish);
    
    clock_gettime(CLOCK_REALTIME, &start);
    std::vector<int, Mallocator<int>> vec_a_i(N); 
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector int alloc allocator time");
    print_time(str_log, start, finish);
    
    clock_gettime(CLOCK_REALTIME, &start);
    std::vector<int, numa_Mallocator<int>> vec_n_a_i(N); 
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector int numa_alloc allocator time");
    print_time(str_log, start, finish);

    clock_gettime(CLOCK_REALTIME, &start);
    #pragma omp parallel for
    for(int i = 0; i < N; ++i) {
        vec[i] = i;
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector init std time");
    print_time(str_log, start, finish);

    clock_gettime(CLOCK_REALTIME, &start);
    
    #pragma omp parallel for
    for(int i = 0; i < N; ++i) {
        vec_a[i] = i;
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector init allocator time");
    print_time(str_log, start, finish);

    for(int i = 0; i < N; ++i) {
    	out_vec += vec[i];    
    }
    for(int i = 0; i < N; ++i) {
    	out_vec_a += vec_a[i];    
    }
    cout << " out_vec " << out_vec << endl;
    cout << " out_vec_a " << out_vec_a << endl;
    
    double out_vec_omp=0, out_vec_a_omp=0;

    clock_gettime(CLOCK_REALTIME, &start);
    #pragma omp parallel for reduction(+:out_vec_omp)
    for(int i = 0; i < N; ++i) {
    	out_vec_omp += vec[i];    
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector add std time");
    print_time(str_log, start, finish);

    clock_gettime(CLOCK_REALTIME, &start);
    #pragma omp parallel for reduction(+:out_vec_a_omp)
    for(int i = 0; i < N; ++i) {
    	out_vec_a_omp += vec_a[i];    
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sprintf(str_log, "Vector add allocator time");
    print_time(str_log, start, finish);
    cout << " out_vec_omp " << out_vec_omp << endl;
    cout << " out_vec_a_omp " << out_vec_a_omp << endl;




    return 0;
}
