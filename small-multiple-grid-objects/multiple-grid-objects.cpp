// Multiple access patterns 
// Regular random - accesses to random number of adjacent element
// Random - random elements within the whole array
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
#include <numa.h>
#include <numaif.h>


#define TYPE double
void  __attribute__ ((noinline)) init_elem ( TYPE *ar1, uint64_t arCnt, TYPE inValue){
  	#pragma omp parallel for 
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) =  ( (i%8) + inValue);
	 }
}

void init_indices_Reg_Rand ( uint64_t  *ar1, uint64_t arCnt, uint64_t numLayers) {
  #pragma omp parallel
  {
    uint64_t thread_id = omp_get_thread_num();
	  uint64_t total_threads = omp_get_num_threads();
	  //std::random_device rd;  // a seed source for the random number engine
 	  //std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::mt19937 gen(1337 + thread_id);
 	  std::uniform_int_distribution<> distrib(1, (numLayers));

    //printf("Hello World from thread %lu out of %lu\n", thread_id, total_threads);
    // Calculate a workload range for this specific thread
	  uint64_t chunk_size = arCnt / total_threads;
	  uint64_t start_idx = thread_id * chunk_size;
	  uint64_t end_idx = (thread_id == total_threads - 1) ? arCnt : start_idx + chunk_size;
	
	  uint64_t idx_chunk_size = arCnt*numLayers / total_threads;
	  uint64_t idx_start_idx = (thread_id) * idx_chunk_size;
    //printf("Hello thread %lu total %lu chunk size %lu start_idx_chunk %lu\n", thread_id,(arCnt*numLayers), 
    //                            idx_chunk_size, idx_start_idx);

    // Process only this thread's portion
	  ar1[start_idx] = idx_start_idx;
    uint64_t prevValue = idx_start_idx;
    if (thread_id != (total_threads -1) ){
      for (uint64_t i = start_idx+1; i < end_idx; i++) {
		    uint64_t randValue = distrib(gen); 
        //ar1[i] = (ar1[i-1]+ randValue) <  (arCnt*numLayers) ?  (ar1[i-1]+ randValue) : (  (arCnt*numLayers) -1 );
        ar1[i] = (prevValue+ randValue) ; // <  (arCnt*numLayers) ?  (prevValue+ randValue) : (  (arCnt*numLayers) -1 );
        prevValue = ar1[i];
      }
    } else {
      for (uint64_t i = start_idx+1; i < end_idx; i++) {
		    uint64_t randValue = distrib(gen); 
        //ar1[i] = (ar1[i-1]+ randValue) <  (arCnt*numLayers) ?  (ar1[i-1]+ randValue) : (  (arCnt*numLayers) -1 );
        ar1[i] = (prevValue+ randValue) <  (arCnt*numLayers) ?  (prevValue+ randValue) : (  (arCnt*numLayers) -1 );
        prevValue = ar1[i];
      }
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
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
	  std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(1337 + id);
    uint64_t randStart=0;
    uint64_t randEnd = arCnt-1;
    //int total = omp_get_num_threads();
    /*if (id <= (total/2)) {
    randStart=0;
    randEnd = (arCnt/2)-1;
    }
   else {
      randStart = (arCnt/2);
      randEnd = arCnt-1;
    } */

 	  std::uniform_int_distribution<> distrib(randStart, randEnd);

 	  #pragma omp for 
	  for(uint64_t i=0; i<arCnt; i++) {
	 	  *(ar1+i) =  distrib(gen);
	  }
  }
	//for(uint64_t i=0; i<arCnt; i++) 
	//printf("Rand Indices %lu  size %lu \n", *(ar1+i), arCnt);
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

void __attribute__ ((noinline)) funSMCalc(TYPE  *out_SurfMoist, uint64_t numElements, TYPE *root_Reg_Rand, 
        uint64_t *root_Reg_Rand_Index, 
				TYPE *soil_Reg_Reuse, uint64_t numSoilLayers, TYPE *veg_Reg_Rand, uint64_t *veg_Reg_Rand_Index, 
				TYPE *frac_SurfMoist_Rand, uint64_t *ar_SurfMoist_Index)
{
	#ifdef DEBUG
	  printf(" in funSMCalc \n");
  #endif
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
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5) / cntSoil;	
			}
		}
    outVeg=0;
		for (vegIndex=*(veg_Reg_Rand_Index+i); vegIndex < (*(veg_Reg_Rand_Index+i+1)); vegIndex++) {
			outVeg += *(veg_Reg_Rand+vegIndex) * 0.25;
		}
		leftSM = *(frac_SurfMoist_Rand+(*(ar_SurfMoist_Index+i))) * 0.25;
		*(out_SurfMoist+i) = 	outSoil * outVeg* leftSM;
	}
}

void __attribute__ ((noinline)) funPrecCalc(TYPE *out_PrecLeft, uint64_t numElements, TYPE *root_Reg_Rand, 
    uint64_t *root_Reg_Rand_Index, TYPE *soil_Reg_Reuse, uint64_t numSoilLayers, TYPE *frac_Prec_Rand, uint64_t *ar_Prec_Index)  
{
	#ifdef DEBUG
    printf(" in funPrecCalc \n");
  #endif
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
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5) / cntSoil;	
			}
		}
    outRoot=0;
		for (rootIndex=*(root_Reg_Rand_Index+i); rootIndex < (*(root_Reg_Rand_Index+i+1)); rootIndex++) {
			outRoot += *(root_Reg_Rand+rootIndex) * 0.25;
		}
		leftPrec = *(frac_Prec_Rand+(*(ar_Prec_Index+i))) * 0.25;
		*(out_PrecLeft+i) = 	outSoil * outRoot * leftPrec;
	}
}

void __attribute__ ((noinline))	funEvapCalc(TYPE *out_EvapOut, uint64_t numElements, TYPE *canopy_Reg_Rand, 
    uint64_t *canopy_Reg_Rand_Index, TYPE *soil_Reg_Reuse, uint64_t numSoilLayers, TYPE *frac_Evap_Rand, uint64_t *ar_Evap_Index)
{
	#ifdef DEBUG
	  printf(" in funEvapCalc \n");
  #endif
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
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5) / cntSoil;	
			}
		}
    outCanopy=0;
		for (canopyIndex=*(canopy_Reg_Rand_Index+i); canopyIndex < (*(canopy_Reg_Rand_Index+i+1)); canopyIndex++) {
			outCanopy += *(canopy_Reg_Rand+canopyIndex) * 0.25;
		}
		leftEvap = *(frac_Evap_Rand+(*(ar_Evap_Index+i))) * 0.25;
		*(out_EvapOut+i) = 	outSoil * outCanopy * leftEvap;
	}
}

void __attribute__ ((noinline))	funAtmosCalc(TYPE *out_AtmosEffect, uint64_t numElements,  
                TYPE *soil_Reg_Reuse, uint64_t numSoilLayers, TYPE *atmos_Reg, uint64_t numAtmosValues)
{
	#ifdef DEBUG
	  printf(" in funAtmosCalc \n");
  #endif
  uint64_t i=0, j=0, k=0;
	TYPE outSoil =0;
	TYPE outAtmos =0;
  #pragma omp parallel for private(i, j, k, outSoil, outAtmos)
  for(i=0; i<numElements; i++) {
		outSoil =0;
		outAtmos=0;
		for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5); 
		}
		for (k=0; k<numAtmosValues; k++) {
				outAtmos += (*(atmos_Reg+i+k)) * (0.5); 
		}
		*(out_AtmosEffect+i) = 	outSoil * outAtmos ;
	}
}	

int main(int argc, char *argv[]) 
{
  // Grid size
  uint64_t numLat = 32*512;
 	uint64_t numLon = 32*512;
 	//uint64_t numLat = 40;
 	//uint64_t numLon = 20;
	uint64_t numVegBands = 32;
	uint64_t numRootLayers = 64;
	uint64_t numSoilLayers = 8;
	uint64_t numCanopyLayers = 4;
	uint64_t numAtmosValues= 8;
	
  struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
	char *str_run_option=(char *) malloc(500*sizeof(char)); 

  int opt;
  char *farMemData = NULL;
  if (argc <  2) {
        fprintf(stderr, "Usage: %s [-h] [-f reg|rand|both]\n", argv[0]);
  }
  // ':' after 'f' means it requires an argument
  while ((opt = getopt(argc, argv, "hf:")) != -1) {
    switch (opt) {
        case 'h':
            printf("Usage: %s [-h] [-f reg|rand|both]\n", argv[0]);
            break;
        case 'f':
            farMemData = optarg; // optarg stores the option's value
            printf("Far memory object is access pattern is : %s \n", farMemData);
            break;
        default:
            fprintf(stderr, "Unknown option.\n");
            return 1;
    }
  }
  int farMemOption=0; 

  if (farMemData == NULL) {
    printf("All critical data allocated in local memory \n");
    sprintf(str_run_option, "RUNTIME All local:");
  } else if (strcmp(farMemData, "reg") == 0) {
    printf("Far memory object has regular access \n");
    farMemOption=1;
    sprintf(str_run_option, "RUNTIME Reg-Rand Far: ");
  } else if (strcmp(farMemData, "rand") == 0) {
    printf("Far memory object has random access \n");
    farMemOption=2;
    sprintf(str_run_option, "RUNTIME Random Far");
  } else if (strcmp(farMemData, "both") == 0) {
    printf("Far memory object has random  and regular access \n");
    farMemOption=3;
    sprintf(str_run_option, "RUNTIME Both Far");
  }

  int total ;
  total = omp_get_max_threads();
  std::cout << "number of threads " <<  total << "\n";

	int numa_node = -1;
  numa_node = numa_preferred();
  int local_numa = numa_preferred();
  int far_numa = (local_numa == 0) ? 1 :0; 
	printf("NUMA preferred %d local %d far %d \n", numa_node, local_numa, far_numa);

  /* START Check numa_alloc functionality code 
  int *mats = (int *) malloc( 1024 * sizeof(int));
  *mats = 1; 
  get_mempolicy(&numa_node, NULL, 0, (void*)mats, MPOL_F_NODE | MPOL_F_ADDR);
	printf("numa_node mats %d \n", numa_node);

  int *mats_0 = (int *) numa_alloc_onnode( 1024 * sizeof(int), 0);
  *mats_0 = 2; 
  get_mempolicy(&numa_node, NULL, 0, (void*)mats_0, MPOL_F_NODE | MPOL_F_ADDR);
	printf("numa_node mats_0 %d \n", numa_node);
 
 int *mats_1 = (int *) numa_alloc_onnode( 1024 * sizeof(int), 1);
  *mats_1 = 2; 
  get_mempolicy(&numa_node, NULL, 0, (void*)mats_1, MPOL_F_NODE | MPOL_F_ADDR);
	printf("numa_node mats_1 %d \n", numa_node);
  // END Check numa_alloc functionality code 
  */
  
 	/* Regular Random - using random number of adjacent elements */	
	TYPE *veg_Reg_Rand ;
	TYPE *root_Reg_Rand ;

 	/* Random */	
	TYPE *frac_SurfMoist_Rand ;
	TYPE *frac_Prec_Rand; // = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));

  if ( farMemOption == 0 ) {
	  frac_SurfMoist_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), local_numa);
	  frac_Prec_Rand = (TYPE *)numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), local_numa);
	  veg_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numVegBands)*sizeof(TYPE), local_numa);
	  root_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numRootLayers)*sizeof(TYPE), local_numa);
  }
  if ( farMemOption == 1 ) {
	  frac_SurfMoist_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), local_numa);
	  frac_Prec_Rand = (TYPE *)numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), local_numa);
	  veg_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numVegBands)*sizeof(TYPE), far_numa);
	  root_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numRootLayers)*sizeof(TYPE), far_numa);
  }
  if ( farMemOption == 2 ) {
	  frac_SurfMoist_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), far_numa);
	  frac_Prec_Rand = (TYPE *)numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), far_numa);
	  veg_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numVegBands)*sizeof(TYPE), local_numa);
	  root_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numRootLayers)*sizeof(TYPE), local_numa);
  }
  if ( farMemOption == 3 ) {
	  frac_SurfMoist_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), far_numa);
	  frac_Prec_Rand = (TYPE *)numa_alloc_onnode (( numLat*numLon)*sizeof(TYPE), far_numa);
	  veg_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numVegBands)*sizeof(TYPE), far_numa);
	  root_Reg_Rand = (TYPE *) numa_alloc_onnode (( numLat*numLon*numRootLayers)*sizeof(TYPE), far_numa);
  }
	
	/* Regular Random Indices */	
	uint64_t *veg_Reg_Rand_Index = (uint64_t *)malloc (( numLat*numLon)*sizeof(TYPE));
	uint64_t *root_Reg_Rand_Index = (uint64_t *)malloc (( numLat*numLon)*sizeof(TYPE));

 	/* Regular with reuse */	
	TYPE *soil_Reg_Reuse = (TYPE *)malloc (( numLat*numLon*numSoilLayers)*sizeof(TYPE));

 	/* Regular with NO reuse */	
	TYPE *atmos_Reg = (TYPE *)malloc (( numLat*numLon*numAtmosValues)*sizeof(TYPE));

 	/* Random Indices */	
	uint64_t *ar_SurfMoist_Index = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));
	uint64_t *ar_Prec_Index = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));
	
  TYPE *out_SurfMoist = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_PrecLeft = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_AtmosEffect = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_Moisture = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
  
  #ifdef DEBUG 
    printf("start init_elem \n");
	#endif
  init_elem(veg_Reg_Rand, numLat*numLon*numVegBands, 0.35);
  get_mempolicy(&numa_node, NULL, 0, (void*)veg_Reg_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("NUMA veg_Reg_Rand %d ", numa_node);
	init_elem(root_Reg_Rand, numLat*numLon*numRootLayers, 0.678);
  get_mempolicy(&numa_node, NULL, 0, (void*)root_Reg_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("root_Reg_Rand %d ", numa_node);
	init_elem(frac_SurfMoist_Rand, numLat*numLon, 0.375);
  get_mempolicy(&numa_node, NULL, 0, (void*)frac_SurfMoist_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("frac_SurfMoist_Rand %d ", numa_node);
	init_elem(frac_Prec_Rand, numLat*numLon, 0.345);
  get_mempolicy(&numa_node, NULL, 0, (void*)frac_Prec_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("frac_Prec_Rand %d \n", numa_node);

	init_elem(soil_Reg_Reuse, numLat*numLon*numSoilLayers, 0.355);
  get_mempolicy(&numa_node, NULL, 0, (void*)soil_Reg_Reuse, MPOL_F_NODE | MPOL_F_ADDR);
	printf("MALLOC soil_Reg_Reuse %d ", numa_node);
	init_elem(atmos_Reg, numLat*numLon*numAtmosValues, 0.365);
  get_mempolicy(&numa_node, NULL, 0, (void*)atmos_Reg, MPOL_F_NODE | MPOL_F_ADDR);
	printf("atmos_Reg %d ", numa_node);

	
  #ifdef DEBUG 
    printf("start init_indices \n");
  #endif
  init_indices_Reg_Rand(veg_Reg_Rand_Index , ( numLat*numLon), numVegBands);
  get_mempolicy(&numa_node, NULL, 0, (void*)veg_Reg_Rand_Index, MPOL_F_NODE | MPOL_F_ADDR);
	printf("veg_Reg_Rand_Index %d \n", numa_node);

	clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Random(ar_SurfMoist_Index, numLat*numLon);
	clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)ar_SurfMoist_Index, MPOL_F_NODE | MPOL_F_ADDR);
	printf("MALLOC ar_SurfMoist_Index %d ", numa_node);
	
  clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Random(ar_Prec_Index, numLat*numLon);
  /* copy random in reverse order if needed
  #pragma omp parallel for
    for (uint64_t j=0; j<(numLat*numLon); j++) {
      *(ar_Prec_Index+j) = *(ar_SurfMoist_Index+ (numLat*numLon)-1 -j);
	} */
  clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)ar_Prec_Index, MPOL_F_NODE | MPOL_F_ADDR);
	printf("ar_Prec_Index %d ", numa_node);

	clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Reg_Rand(root_Reg_Rand_Index,  ( numLat*numLon), numRootLayers); 
	clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init reg rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)root_Reg_Rand_Index, MPOL_F_NODE | MPOL_F_ADDR);
	printf("root_Reg_Rand_Index %d \n", numa_node);

	clock_gettime(CLOCK_REALTIME, &start); 
	for ( int loopCnt=0; loopCnt <2; loopCnt++) {
	  funSMCalc(out_SurfMoist, (numLat*numLon), root_Reg_Rand, root_Reg_Rand_Index, soil_Reg_Reuse, numSoilLayers, 
            veg_Reg_Rand, veg_Reg_Rand_Index, frac_SurfMoist_Rand, ar_SurfMoist_Index);  
	  funPrecCalc(out_PrecLeft, (numLat*numLon), root_Reg_Rand, root_Reg_Rand_Index, soil_Reg_Reuse, numSoilLayers, 
              frac_Prec_Rand, ar_Prec_Index);  
	  funAtmosCalc(out_AtmosEffect, (numLat*numLon),  soil_Reg_Reuse, numSoilLayers, atmos_Reg, numAtmosValues);  
	}
	clock_gettime(CLOCK_REALTIME, &finish); 

	uint64_t i=0;
	#pragma omp parallel for private (i) 
	for(i=0; i<( numLat*numLon); i++) {
		*(out_Moisture+i) = *(out_SurfMoist+i ) + *(out_PrecLeft+i) -  *(out_AtmosEffect+i);
	}
  get_mempolicy(&numa_node, NULL, 0, (void*)out_SurfMoist, MPOL_F_NODE | MPOL_F_ADDR);
	printf("MALLOC out_SurfMoist %d ", numa_node);
  get_mempolicy(&numa_node, NULL, 0, (void*)out_PrecLeft, MPOL_F_NODE | MPOL_F_ADDR);
	printf("out_PrecLeft %d ", numa_node);
  get_mempolicy(&numa_node, NULL, 0, (void*)out_AtmosEffect, MPOL_F_NODE | MPOL_F_ADDR);
	printf("out_AtmosEffect %d ", numa_node);
  get_mempolicy(&numa_node, NULL, 0, (void*)out_Moisture, MPOL_F_NODE | MPOL_F_ADDR);
  printf("out_Moisture %d \n", numa_node);

	printf("output values om %f %f ", *(out_Moisture+1), *(out_Moisture+((numLat*numLon)-1)));
	printf("sm %f %f \n", *(out_SurfMoist+1), *(out_SurfMoist+((numLat*numLon)-1)));
	printf("prec %f %f ", *(out_PrecLeft+1), *(out_PrecLeft+((numLat*numLon)-1)));
	printf("atmos %f %f \n", *(out_AtmosEffect+1), *(out_AtmosEffect+((numLat*numLon)-1)));
	
  print_time(str_run_option, start, finish);
  return 0;
}
