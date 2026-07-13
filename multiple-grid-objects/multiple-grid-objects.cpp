
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




int main(void) {
	struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
  	uint64_t numLat = 512*512;
  	uint64_t numLon = 256*256;
	uint64_t numVegBands = 4;
	uint64_t numRootLayers = 4;
	uint64_t numSoilLayers = 5;
	uint64_t numCanopyLayers = 3;
	uint64_t numAtmosValues= 10;

 	/* Regular Random */	
	TYPE *veg_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numVegBands)*sizeof(TYPE));
	TYPE *root_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numRootLayers)*sizeof(TYPE));
	TYPE *canopy_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numCanopyLayers)*sizeof(TYPE));

 	/* Regular with reuse */	
	TYPE *soil_Reg = (TYPE *)malloc (( numLat*numLon*numSoilLayers)*sizeof(TYPE));

 	/* Regular with NO reuse */	
	TYPE *atmos_Reg = (TYPE *)malloc (( numLat*numLon*numAtmosValues)*sizeof(TYPE));

 	/* Random */	
	TYPE *frac_SurfMoist_Rand = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *frac_Prec_Rand = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *frac_Evap_Rand = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	
 	/* Random Indices */	
	uint64_t *ar_SurfMoist_Index = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));
	uint64_t *ar_Prec_Index = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));
	uint64_t *ar_Evap_Index = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));

	init_elem_Reg_Rand(veg_Reg_Rand, numLat*numLon*numVegBands, numVegBands);
	init_elem_Reg_Rand(root_Reg_Rand, numLat*numLon*numRootLayers, numRootLayers);
	init_elem_Reg_Rand(veg_Reg_Rand, numLat*numLon*numCanopyLayers, numCanopyLayers);

	init_elem_Reg(soil_Reg, numLat*numLon*numSoilLayers);
	init_elem_Reg(atmos_Reg, numLat*numLon*numAtmosValues);

	init_elem_Reg(frac_SurfMoist_Rand, numLat*numLon);
	init_elem_Reg(frac_Prec_Rand, numLat*numLon);
	init_elem_Reg(frac_Evap_Rand, numLat*numLon);

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
