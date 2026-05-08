
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "print_time.h"
#include <time.h>
#include <omp.h>
#include <sched.h>
#include <iostream>
#include <random>

#define TYPE double
#define sizeRatio 4
uint16_t stride=512;


void  __attribute__ ((noinline)) init_elem ( TYPE *ar1, uint64_t arCnt){
  	#pragma omp parallel for 
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) = ( (10000+i + (i*2)) % arCnt);
	 }
}

void  __attribute__ ((noinline)) init_indices ( uint64_t *ar1, uint64_t arCnt){
	srand(time(NULL));
	#pragma omp parallel
	{
		std::random_device rd;  // a seed source for the random number engine
    		std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    		std::uniform_int_distribution<> distrib(0, (arCnt-1));

    		/*for (int n = 0; n != 10; ++n)
        		std::cout << distrib(gen) << ' ';
    		*/
  		#pragma omp parallel for 
		for(uint64_t i=0; i<arCnt; i++) {
		 	//*(ar1+i) =  rand() % (arCnt);
		 	//*(ar1+i) =  i;
		 	*(ar1+i) =  distrib(gen);
		}
	}
}

TYPE __attribute__ ((noinline)) sum_arr1_index_arr2(TYPE *ar1, uint64_t *ar2, uint64_t arCnt){
	TYPE check_sum=0;
	uint64_t j=0;
	for(uint64_t i=0; i<arCnt; i++) {
		j = *(ar2+i);
		check_sum += (*(ar1+j));
	}
	return check_sum;
}
   
TYPE __attribute__ ((noinline)) sum_arr_index_strided(TYPE *ar1, uint64_t arCnt){
  TYPE check_sum=0;
  uint64_t i=0, j=0;
  for(i=0; i<=arCnt-stride; i+=stride) {
	  for (j=0; j<stride; j++) { 
      		check_sum+=  ((*(ar1+i))*(*(ar1+i+j)));
	  }
   }
    return check_sum;
}

TYPE __attribute__ ((noinline)) sum_two_arr_index_strided(TYPE *ar1, TYPE *ar2, uint64_t arCnt){
  TYPE check_sum=0;
  uint64_t i=0, j=0;
  for(i=0; i<=arCnt-stride; i+=stride) {
	  for (j=0; j<stride; j++) { 
      		check_sum+=  ((*(ar1+i))*(*(ar1+i+j))) + ((*(ar2+i))*(*(ar2+i+j)));
	  }
   }
    return check_sum;
}

TYPE __attribute__ ((noinline)) sum_three_arr(TYPE *arB, TYPE *arA, TYPE *arC, 
			uint64_t *arSmallIndex, uint64_t *arLargeIndex, 
			uint64_t arSmallSize, uint64_t arLargeSize){
  TYPE check_sum=0;
  uint64_t i=0, j=0, arACIndex, arACIndexNext;
  uint64_t arBIndex=0;
  #pragma omp parallel for reduction(+:check_sum) private(i, j, arBIndex, arACIndex, arACIndexNext) 
  for(i=0; i<arSmallSize; i++) {
	j= i*2;
	arACIndex = *(arLargeIndex+j);
	arACIndexNext = *(arLargeIndex+j+1);
	arBIndex= *(arSmallIndex+i);
	//printf("Indices B %lu  A, C %lu %lu\n", arBIndex, arACIndex, arACIndexNext);
	// BACAC BACAC
      	check_sum+=  (*(arB+arBIndex))* ((*(arA+arACIndex))+ (*(arC+arACIndex))) + ((*(arA+arACIndexNext)* (*(arC+arACIndexNext))));
	}
    return check_sum;
}


TYPE __attribute__ ((noinline)) sum_three_arr_out(TYPE *arB, TYPE *arA, TYPE *arC, TYPE *out, 
			uint64_t *arSmallIndex, uint64_t *arLargeIndex, 
			uint64_t arSmallSize, uint64_t arLargeSize)
{
  TYPE check_sum=0;
  uint64_t i=0, j=0, arACIndex=0, arACIndexNext=0;
  uint64_t arBIndex=0;
  #pragma omp parallel for private( i, j, arBIndex, arACIndex, arACIndexNext) 
  for(i=0; i<arSmallSize; i++) {
	arBIndex= i; //*(arSmallIndex+i);
	j= i*2;
	arACIndex = *(arLargeIndex+j); //j
	arACIndexNext =  *(arLargeIndex+j+1); //j+1
	arBIndex= *(arSmallIndex+i); //i
	/*
	int thread_num = omp_get_thread_num();
        int cpu_num = sched_getcpu();
        printf("Thread %3d is running on CPU %3d\n", thread_num, cpu_num);
	printf("thr %d cpu %d Ind B %lu  A, C %lu %lu i %lu j %lu \n", 
				thread_num, cpu_num, arBIndex, arACIndex, arACIndexNext, i, j);
	sleep(2);
	*/
	// BACAC BACAC
      	*(out+i)=  (*(arB+arBIndex))+ ((*(arA+arACIndex))* (*(arC+arACIndex))) 
				+ ((*(arA+arACIndexNext)* (*(arC+arACIndexNext))));
	}
    return check_sum;
}

TYPE __attribute__ ((noinline)) sum_three_X_arr_out(TYPE *arB, TYPE *arA, TYPE *arC, TYPE *out, 
			uint64_t *arSmallIndex, uint64_t *arLargeIndex, 
			uint64_t arSmallSize, uint64_t arLargeSize)
{
  TYPE check_sum=0;
  uint64_t i=0, j=0, arACIndex=0, arACIndexNext=0;
  uint64_t arBIndex=0;
  #pragma omp parallel for private( i, j, arBIndex, arACIndex, arACIndexNext) 
  for(i=0; i<arSmallSize; i++) {
	arBIndex= i; //*(arSmallIndex+i);
	j= i*2;
	arACIndex = *(arLargeIndex+j); //j
	arACIndexNext =  *(arLargeIndex+j+1); //j+1
	arBIndex= *(arSmallIndex+i); //i
	// ACACACAC.....ACXACACACAC....ACXACACACAC...
      	*(out+i)=   ((*(arA+arACIndex))* (*(arC+arACIndex))) 
				+ ((*(arA+arACIndexNext)* (*(arC+arACIndexNext))));
	if(i % 64 == 0) 
		*(out+i) = (*(arB+arBIndex));
	}
    return check_sum;
}


TYPE __attribute__ ((noinline)) sum_two_arr(TYPE *arA, TYPE *arC, 
			uint64_t *arLargeIndex, 
			uint64_t arLargeSize){
  TYPE check_sum=0;
  uint64_t  j=0, arACIndex, arACIndexNext;
  #pragma omp parallel for reduction(+:check_sum) private (j, arACIndex, arACIndexNext) 
  for(j=0; j<arLargeSize; j+=2) {
	arACIndex = *(arLargeIndex+j);
	arACIndexNext = *(arLargeIndex+j+1);
	// ACACACAC
      	check_sum+=   ((*(arA+arACIndex))+ (*(arC+arACIndex))) + ((*(arA+arACIndexNext)* (*(arC+arACIndexNext))));
	}
    return check_sum;
}

TYPE __attribute__ ((noinline)) sum_two_arr_out(TYPE *arA, TYPE *arC, TYPE *ar_Out,  
			uint64_t *arLargeIndex, 
			uint64_t arLargeSize)
{
  TYPE check_sum=0;
  uint64_t  j=0, arACIndex; 
  #pragma omp parallel for private(arACIndex)
  for(j=0; j<arLargeSize; j++) {
	arACIndex = *(arLargeIndex+j); //j
	// ACACACAC
      	*(ar_Out+j)=   ((*(arA+arACIndex))+ (*(arC+arACIndex))); 
	}
    return check_sum;
}

int main(void) {
	struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
  // L3 - Unified Junction 32768K - 1/2 is 2*1024*1024 doubles
  // L1 - Data 32K - half is 2048 (8 bytes)
  //
  uint64_t arSizeSmall = 2*1024*512*512;
  uint64_t arSizeLarge = arSizeSmall*2;
  int loopCnt = 1;
	
	TYPE *ar_A = (TYPE *)malloc ((arSizeLarge)*sizeof(TYPE));
	TYPE *ar_C = (TYPE *)malloc ((arSizeLarge)*sizeof(TYPE));
	//TYPE *ar_E = (TYPE *)malloc ((arSizeLarge)*sizeof(TYPE));
	TYPE *ar_B = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_D = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_X = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Y = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Out_Sm1 = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Out_Sm2 = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Out_Sm3 = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Out_Sm4 = (TYPE *)malloc ((arSizeSmall)*sizeof(TYPE));
	TYPE *ar_Out_Large = (TYPE *)malloc ((arSizeLarge)*sizeof(TYPE));

	init_elem(ar_A, arSizeLarge);
	//init_elem(ar_E, arSizeLarge);
	init_elem(ar_B, arSizeSmall);
	init_elem(ar_C, arSizeLarge);
	init_elem(ar_D, arSizeSmall);
	init_elem(ar_X, arSizeSmall);
	init_elem(ar_Y, arSizeSmall);

	srand(time(NULL)); 
	clock_gettime(CLOCK_REALTIME, &start); 
	uint64_t *ar_Small_Index = (uint64_t *)malloc ((arSizeSmall)*sizeof(uint64_t));
	init_indices(ar_Small_Index, arSizeSmall);
	uint64_t *ar_Large_Index = (uint64_t *)malloc ((arSizeLarge)*sizeof(uint64_t));
	init_indices(ar_Large_Index, arSizeLarge);
	
	clock_gettime(CLOCK_REALTIME, &finish); 
	sprintf(str_log, "Array init indices time");  
	print_time(str_log, start, finish);

	TYPE resSmall=0;
	TYPE resLarge=0;
	TYPE resLarge1=0;
	TYPE resSmall1=0;
	sprintf(str_log, "Array Run time");  
	clock_gettime(CLOCK_REALTIME, &start); 

  	for (int i=0; i< loopCnt; i++) {	
		//printf("i - %d \n", i);
		//resSmall += sum_three_arr( ar_B, ar_A, ar_C, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
		//resLarge += sum_two_arr( ar_A, ar_C, ar_Large_Index,  arSizeLarge);
		//resSmall1 += sum_three_arr( ar_D, ar_A, ar_C, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
		resLarge1 += sum_three_arr_out( ar_B, ar_A, ar_C, ar_Out_Sm1, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
		resLarge += sum_two_arr_out( ar_A, ar_C, ar_Out_Large, ar_Large_Index,  arSizeLarge);
		resLarge1 += sum_three_X_arr_out( ar_X, ar_A, ar_C, ar_Out_Sm3, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
		resLarge1 += sum_three_arr_out( ar_D, ar_C, ar_A, ar_Out_Sm2, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
		resLarge1 += sum_three_X_arr_out( ar_Y, ar_C, ar_A, ar_Out_Sm4, ar_Small_Index, ar_Large_Index, arSizeSmall, arSizeLarge);
	}
	clock_gettime(CLOCK_REALTIME, &finish); 
	printf("Small %lf ", resSmall);
	printf("Large %lf ", resLarge);
	printf("Large1 %lf ", resLarge1);
	printf("Small1 %lf \n", resSmall1);
	printf("Array values %lf %lf \n", *(ar_Out_Sm3+1), *(ar_Out_Sm3+2));
	printf("Array values %lf %lf \n", *(ar_Out_Sm2+2), *(ar_Out_Sm2+30));
	printf("Array values %lf %lf \n", *(ar_Out_Sm1+1), *(ar_Out_Sm1+20));
	printf("Array values %lf %lf \n", *(ar_Out_Large+1), *(ar_Out_Large+2));
                                       
	print_time(str_log, start, finish);
  free(ar_Small_Index);
  return 0;
}
