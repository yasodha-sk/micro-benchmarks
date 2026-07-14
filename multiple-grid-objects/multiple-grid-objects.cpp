
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


void  __attribute__ ((noinline)) init_elem ( TYPE *ar1, uint64_t arCnt, TYPE inValue){
  	#pragma omp parallel for 
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) =  ( (i%8) + inValue);
	 }
}

void init_indices_Reg_Rand ( uint64_t  *ar1, uint64_t arCnt, uint64_t numLayers) {
srand(time(NULL));
#pragma omp parallel
{
	std::random_device rd;  // a seed source for the random number engine
    	std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    	std::uniform_int_distribution<> distrib(1, (numLayers));
	uint64_t thread_id = omp_get_thread_num();
	uint64_t total_threads = omp_get_num_threads();
 //printf("Hello World from thread %lu out of %lu\n", thread_id, total_threads);
	 // Calculate a workload range for this specific thread
	uint64_t chunk_size = arCnt / total_threads;
	uint64_t start_idx = thread_id * chunk_size;
	uint64_t end_idx = (thread_id == total_threads - 1) ? arCnt : start_idx + chunk_size;
	
	uint64_t idx_chunk_size = arCnt*numLayers / total_threads;
	uint64_t idx_start_idx = (thread_id) * idx_chunk_size;
 //printf("Hello World from thread %lu total %lu chunk size %lu start_idx_chunk %lu\n", thread_id,(arCnt*numLayers), idx_chunk_size, idx_start_idx);

         // Process only this thread's portion
	 ar1[start_idx] = idx_start_idx;  
         for (uint64_t i = start_idx+1; i < end_idx; i++) {
		 uint64_t randValue = distrib(gen); 
                 ar1[i] = (ar1[i-1]+ randValue) <  (arCnt*numLayers) ?  (ar1[i-1]+ randValue) : (  (arCnt*numLayers) -1 );
         }

}
/* Sequential version
	 *(ar1)=0;
         for (uint64_t i = 1; i < arCnt; i++) {
	 	*(ar1+i) =  *(ar1+i-1) + distrib(gen);
         }
*/
	
	//for(uint64_t i=0; i<arCnt; i++) 
	//printf("Reg Rand Indices %lu  size %lu \n", *(ar1+i), arCnt*numLayers);

}

void  __attribute__ ((noinline)) init_indices_Random ( uint64_t *ar1, uint64_t arCnt){
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
	//for(uint64_t i=0; i<arCnt; i++) 
	printf("Rand Indices %lu  size %lu \n", *(ar1+arCnt-1), arCnt);
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

void __attribute__ ((noinline)) funSMCalc(TYPE  *out_SurfMoist, uint64_t numElements, TYPE *root_Reg_Rand, uint64_t *root_Reg_Rand_Index, 
				TYPE *soil_Reg, uint64_t numSoilLayers, TYPE *veg_Reg_Rand, uint64_t *veg_Reg_Rand_Index, 
				TYPE *frac_SurfMoist_Rand, uint64_t *ar_SurfMoist_Index)
{
	printf(" in funSMCalc \n");
  uint64_t i=0, j=0, k=0;
	uint64_t vegIndex =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outVeg =0;
	TYPE leftSM =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outVeg, vegIndex, leftSM)
  for(i=0; i<numElements; i++) {
		cntSoil = i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg+i+j)) * (0.5) / cntSoil;	
			}
		}
		for (vegIndex=*(veg_Reg_Rand_Index+i); vegIndex < (*(veg_Reg_Rand_Index+i+1)); vegIndex++) 
		{
			outVeg += *(veg_Reg_Rand+vegIndex) * 0.25;
		}
		leftSM = *(frac_SurfMoist_Rand+(*(ar_SurfMoist_Index+i))) * 0.25;

		*(out_SurfMoist+i) = 	outSoil * outVeg* leftSM;
	}
}

void __attribute__ ((noinline)) funPrecCalc(TYPE *out_PrecLeft, uint64_t numElements, TYPE *root_Reg_Rand, uint64_t *root_Reg_Rand_Index, TYPE *soil_Reg, uint64_t numSoilLayers, TYPE *frac_Prec_Rand, uint64_t *ar_Prec_Index)  
{
	printf(" in funPrecCalc \n");
  uint64_t i=0, j=0, k=0;
	uint64_t rootIndex =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outRoot =0;
	TYPE leftPrec =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outRoot, rootIndex, leftPrec)
  for(i=0; i<numElements; i++) {
		cntSoil = i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg+i+j)) * (0.5) / cntSoil;	
			}
		}
		for (rootIndex=*(root_Reg_Rand_Index+i); rootIndex < (*(root_Reg_Rand_Index+i+1)); rootIndex++) 
		{
			outRoot += *(root_Reg_Rand+rootIndex) * 0.25;
		}
		leftPrec = *(frac_Prec_Rand+(*(ar_Prec_Index+i))) * 0.25;

		*(out_PrecLeft+i) = 	outSoil * outRoot * leftPrec;
	}
}

void __attribute__ ((noinline))	funEvapCalc(TYPE *out_EvapOut, uint64_t numElements, TYPE *canopy_Reg_Rand, uint64_t *canopy_Reg_Rand_Index, TYPE *soil_Reg, uint64_t numSoilLayers, TYPE *frac_Evap_Rand, uint64_t *ar_Evap_Index)
{
	printf(" in funEvapCalc \n");
  uint64_t i=0, j=0, k=0;
	uint64_t canopyIndex =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outCanopy =0;
	TYPE leftEvap =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outCanopy, canopyIndex, leftEvap)
  for(i=0; i<numElements; i++) {
		cntSoil = i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg+i+j)) * (0.5) / cntSoil;	
			}
		}
		for (canopyIndex=*(canopy_Reg_Rand_Index+i); canopyIndex < (*(canopy_Reg_Rand_Index+i+1)); canopyIndex++) 
		{
			outCanopy += *(canopy_Reg_Rand+canopyIndex) * 0.25;
		}
		leftEvap = *(frac_Evap_Rand+(*(ar_Evap_Index+i))) * 0.25;

		*(out_EvapOut+i) = 	outSoil * outCanopy * leftEvap;
	}
}

void __attribute__ ((noinline))	funAtmosCalc(TYPE *out_AtmosEffect, uint64_t numElements,  TYPE *soil_Reg, uint64_t numSoilLayers, TYPE *atmos_Reg, uint64_t numAtmosValues)
{
	printf(" in funAtmosCalc \n");
  uint64_t i=0, j=0, k=0;
	TYPE outSoil =0;
	TYPE outAtmos =0;
  #pragma omp parallel for private(i, j, k, outSoil, outAtmos)
  for(i=0; i<numElements; i++) {
		outSoil =0;
		outAtmos=0;
		for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg+i+j)) * (0.5); 
		}
		for (k=0; k<numAtmosValues; k++) {
				outAtmos += (*(atmos_Reg+i+k)) * (0.5); 
		}

		*(out_AtmosEffect+i) = 	outSoil * outAtmos ;
	}

}	



int main(void) {
	struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
  	//uint64_t numLat = 512*512;
  	//uint64_t numLon = 256*256;
  	uint64_t numLat = 40*500;
  	uint64_t numLon = 20*500;
	uint64_t numVegBands = 4;
	uint64_t numRootLayers = 4;
	uint64_t numSoilLayers = 5;
	uint64_t numCanopyLayers = 3;
	uint64_t numAtmosValues= 10;

 	/* Regular Random */	
	TYPE *veg_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numVegBands)*sizeof(TYPE));
	TYPE *root_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numRootLayers)*sizeof(TYPE));
	TYPE *canopy_Reg_Rand = (TYPE *)malloc (( numLat*numLon*numCanopyLayers)*sizeof(TYPE));
	
	/* Regular Random Indices */	
	uint64_t *veg_Reg_Rand_Index = (uint64_t *)malloc (( numLat*numLon)*sizeof(TYPE));
	uint64_t *root_Reg_Rand_Index = (uint64_t *)malloc (( numLat*numLon)*sizeof(TYPE));
	uint64_t *canopy_Reg_Rand_Index = (uint64_t *)malloc (( numLat*numLon)*sizeof(TYPE));

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
 	
	init_elem(veg_Reg_Rand, numLat*numLon*numVegBands, 0.35);
	init_elem(root_Reg_Rand, numLat*numLon*numRootLayers, 0.678);
	init_elem(canopy_Reg_Rand, numLat*numLon*numCanopyLayers, 0.355);

	init_elem(soil_Reg, numLat*numLon*numSoilLayers, 0.355);
	init_elem(atmos_Reg, numLat*numLon*numAtmosValues, 0.365);

	init_elem(frac_SurfMoist_Rand, numLat*numLon, 0.375);
	init_elem(frac_Prec_Rand, numLat*numLon, 0.345);
	init_elem(frac_Evap_Rand, numLat*numLon, 0.335);
	
	srand(time(NULL)); 
	clock_gettime(CLOCK_REALTIME, &start); 
	
	init_indices_Random(ar_SurfMoist_Index, numLat*numLon);
	init_indices_Random(ar_Prec_Index, numLat*numLon);
	init_indices_Random(ar_Evap_Index, numLat*numLon);
	
	init_indices_Reg_Rand(veg_Reg_Rand_Index , ( numLat*numLon), numVegBands);
	init_indices_Reg_Rand(root_Reg_Rand_Index,  ( numLat*numLon), numRootLayers); 
	init_indices_Reg_Rand(canopy_Reg_Rand_Index,  ( numLat*numLon), numCanopyLayers);
	
	clock_gettime(CLOCK_REALTIME, &finish); 
	sprintf(str_log, "init indices time");  
	print_time(str_log, start, finish);

	TYPE *out_SurfMoist = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_PrecLeft = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_EvapOut = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_AtmosEffect = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_Moisture = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	sprintf(str_log, "Run time");  
	clock_gettime(CLOCK_REALTIME, &start); 

	for ( int loopCnt=0; loopCnt <5; loopCnt++) {
	funSMCalc( out_SurfMoist, (numLat*numLon), root_Reg_Rand, root_Reg_Rand_Index, soil_Reg, numSoilLayers, veg_Reg_Rand, veg_Reg_Rand_Index, frac_SurfMoist_Rand, ar_SurfMoist_Index);  
	funPrecCalc( out_PrecLeft, (numLat*numLon), root_Reg_Rand, root_Reg_Rand_Index, soil_Reg, numSoilLayers, frac_Prec_Rand, ar_Prec_Index);  
	funEvapCalc( out_EvapOut, (numLat*numLon), canopy_Reg_Rand, canopy_Reg_Rand_Index, soil_Reg, numSoilLayers, frac_Evap_Rand, ar_Evap_Index);  
	funAtmosCalc( out_AtmosEffect, (numLat*numLon),  soil_Reg, numSoilLayers, atmos_Reg, numAtmosValues);  
	}

	clock_gettime(CLOCK_REALTIME, &finish); 
	print_time(str_log, start, finish);
	uint64_t i=0;
	#pragma omp parallel for private (i) 
	for(i=0; i<( numLat*numLon); i++) {
		*(out_Moisture+i) = *(out_SurfMoist+i ) + *(out_PrecLeft+i) - *(out_EvapOut+i) - *(out_AtmosEffect+i);
	}
	        printf("out moisture 0 %f last %f \n", *out_Moisture, *(out_Moisture+((numLat*numLon)-1)));
  return 0;
}
