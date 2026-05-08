
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "print_time.h"

#define TYPE double
#define sizeRatio 4
uint16_t stride=512;

TYPE __attribute__ ((noinline)) add_elem( TYPE in1, TYPE in2)
{
	TYPE result;
	result = in1+in2;
	return result;
}

void  __attribute__ ((noinline)) init_elem ( TYPE *ar1, uint64_t arCnt)
{
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) = ( (10000+i + (i*2)) % arCnt);
	 }
}

void  __attribute__ ((noinline)) init_indices ( uint64_t *ar1, uint64_t arCnt)
{
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) = ( (i + (i*2)) % arCnt);
	 }
}

TYPE __attribute__ ((noinline)) sum_arr1_index_arr2(TYPE *ar1, uint64_t *ar2, uint64_t arCnt)
{
	TYPE check_sum=0;
	uint64_t j=0;
	for(uint64_t i=0; i<arCnt; i++) {
		j = *(ar2+i);
		check_sum += (*(ar1+j));
	}
	return check_sum;
}
   
TYPE __attribute__ ((noinline)) sum_arr_index_strided(TYPE *ar1, uint64_t arCnt)
{
  TYPE check_sum=0;
  uint64_t i=0, j=0;
  for(i=0; i<=arCnt-stride; i+=stride) {
	  for (j=0; j<stride; j++) { 
      		check_sum+=  ((*(ar1+i))*(*(ar1+i+j)));
	  }
   }
    return check_sum;
}

TYPE __attribute__ ((noinline)) sum_two_arr_index_strided(TYPE *ar1, TYPE *ar2, uint64_t arCnt)
{
  TYPE check_sum=0;
  uint64_t i=0, j=0;
  for(i=0; i<=arCnt-stride; i+=stride) {
	  for (j=0; j<stride; j++) { 
      		check_sum+=  ((*(ar1+i))*(*(ar1+i+j))) + ((*(ar2+i))*(*(ar2+i+j)));
	  }
   }
    return check_sum;
}
int main(void) {
	struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
  // L3 - Unified Junction 32768K - 1/2 is 2*1024*1024 doubles
  //uint64_t arSizeSmall = 2*1024*1024;
  // L3 - Unified Bluesky 19712K - 1/2 is 1232*1024 doubles
  // L1 - Data 32K - half is 2048 (8 bytes)
  //
  //uint64_t arSizeSmall = 2*512*1024*1024;
  uint64_t arSizeSmall = 2*512*1024*512;
  uint64_t arSizeLarge = arSizeSmall;
  int loopCnt = 5;
	
	sprintf(str_log, "Array Run time");  

	TYPE *ar_Small = malloc ((arSizeSmall)*sizeof(TYPE));
	init_elem(ar_Small, arSizeSmall);
	uint64_t *ar_Small_Index = malloc ((arSizeSmall)*sizeof(uint64_t));
	init_indices(ar_Small_Index, arSizeSmall);
	
	TYPE *ar_Small1 = malloc ((arSizeSmall)*sizeof(TYPE));
	init_elem(ar_Small1, arSizeSmall);
	uint64_t *ar_Small_Index1 = malloc ((arSizeSmall)*sizeof(uint64_t));
	init_indices(ar_Small_Index1, arSizeSmall);

	TYPE *ar_Large = malloc ((arSizeLarge)*sizeof(TYPE));
	init_elem(ar_Large, arSizeLarge);
	
	TYPE *ar_Large1 = malloc ((arSizeLarge)*sizeof(TYPE));
	init_elem(ar_Large1, arSizeLarge);
	
	TYPE *ar_Large2 = malloc ((arSizeLarge)*sizeof(TYPE));
	init_elem(ar_Large2, arSizeLarge);

	TYPE resSmall=0;
	TYPE resLarge=0;
	TYPE resLarge1=0;
	TYPE resSmall1=0;
	clock_gettime(CLOCK_REALTIME, &start); 
  	for (int i=0; i< loopCnt; i++) {	
		//printf("i - %d \n");
		resSmall += sum_arr1_index_arr2(ar_Small, ar_Small_Index, arSizeSmall);
		resLarge += sum_arr_index_strided(ar_Large, arSizeLarge);
		resLarge1 += sum_two_arr_index_strided(ar_Large1, ar_Large2, arSizeLarge);
		resSmall1 += sum_arr1_index_arr2(ar_Small1, ar_Small_Index1, arSizeSmall);

	}
	clock_gettime(CLOCK_REALTIME, &finish); 
	printf("Small %lf ", resSmall);
	printf("Large %lf ", resLarge);
	printf("Large1 %lf ", resLarge1);
	printf("Small1 %lf ", resSmall1);
                                       
	print_time(str_log, start, finish);
  free(ar_Small);
  free(ar_Large);
  free(ar_Large1);
  free(ar_Small_Index);
  free(ar_Small_Index1);
  return 0;
}
