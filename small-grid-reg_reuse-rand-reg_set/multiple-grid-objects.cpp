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


int numLoops = 10;
// Grid size
uint64_t numLat = 32*512;
uint64_t numLon = 32*512;
//Grid size that Works for numalloc
//uint64_t numLat = 400;
//uint64_t numLon = 400;
uint64_t numVegBands = 16;
uint64_t numRootLayers = 8;
uint64_t numSoilLayers = 8;
uint64_t numCanopyLayers = 4;
uint64_t numAtmosValues= 8;


#define TYPE double
void  __attribute__ ((noinline)) init_elem ( TYPE *ar1, uint64_t arCnt, TYPE inValue){
  	#pragma omp parallel for 
	 for(uint64_t i=0; i<arCnt; i++) {
		 *(ar1+i) =  ( (i%8) + inValue);
	 }
}

void init_indices_Reg_Set ( uint64_t  *ar1, uint64_t arCnt, uint64_t numLayers) {
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
        ar1[i] = (prevValue+ randValue) ; 
        prevValue = ar1[i];
      }
    } else {
      for (uint64_t i = start_idx+1; i < end_idx; i++) {
		    uint64_t randValue = distrib(gen); 
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
 	  std::uniform_int_distribution<> distrib(randStart, randEnd);

 	  #pragma omp for 
	  for(uint64_t i=0; i<arCnt; i++) {
	 	  *(ar1+i) =  distrib(gen);
	  }
  }
	//for(uint64_t i=0; i<arCnt; i++) 
	//printf("Rand Indices %lu  size %lu \n", *(ar1+i), arCnt);
}


void __attribute__ ((noinline)) funSMCalc(TYPE  *out_SurfMoist, uint64_t numElements, TYPE *root_Reg_Set, 
        uint64_t *root_Reg_Set_Idx, 
				TYPE *soil_Reg_Reuse, TYPE *veg_Reg_Set, uint64_t *veg_Reg_Set_Idx, 
				TYPE *sm_Rand, uint64_t *sm_Rand_Idx)
{
	#ifdef DEBUG
	  printf(" in funSMCalc \n");
  #endif
  uint64_t i=0, j=0, k=0;
	uint64_t vegIdx =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outVeg =0;
	TYPE leftSM =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outVeg, vegIdx, leftSM)
  for(i=0; i<numElements; i++) {
		cntSoil = 6; //i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5) / cntSoil;	
			}
		}
    outVeg=0;
		for (vegIdx=*(veg_Reg_Set_Idx+i); vegIdx < (*(veg_Reg_Set_Idx+i+1)); vegIdx++) {
			outVeg += *(veg_Reg_Set+vegIdx) * 0.33;
		}
		leftSM = *(sm_Rand+(*(sm_Rand_Idx+i))) / 0.25;
		*(out_SurfMoist+i) = 	outSoil * outVeg* leftSM;
	}
}

void __attribute__ ((noinline)) funPrecCalc(TYPE *out_PrecLeft, uint64_t numElements, TYPE *root_Reg_Set, 
    uint64_t *root_Reg_Set_Idx, TYPE *soil_Reg_Reuse, TYPE *prec_Rand, uint64_t *prec_Rand_Idx)  
{
	#ifdef DEBUG
    printf(" in funPrecCalc \n");
  #endif
  uint64_t i=0, j=0, k=0;
	uint64_t rootIdx =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outRoot =0;
	TYPE leftPrec =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outRoot, rootIdx, leftPrec)
  for(i=0; i<numElements; i++) {
		cntSoil = 7; //i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.567) / cntSoil;	
			}
		}
    outRoot=0;
		for (rootIdx=*(root_Reg_Set_Idx+i); rootIdx < (*(root_Reg_Set_Idx+i+1)); rootIdx++) {
			outRoot += *(root_Reg_Set+rootIdx) * 0.2534;
		}
		leftPrec = *(prec_Rand+(*(prec_Rand_Idx+i))) * 0.7853;
		*(out_PrecLeft+i) = 	outSoil * outRoot * leftPrec;
	}
}

void __attribute__ ((noinline))	funEvapCalc(TYPE *out_EvapOut, uint64_t numElements, TYPE *canopy_Reg_Set, 
    uint64_t *canopy_Reg_Set_Idx, TYPE *soil_Reg_Reuse, TYPE *frac_Evap_Rand, uint64_t *ar_Evap_Idx)
{
	#ifdef DEBUG
	  printf(" in funEvapCalc \n");
  #endif
  uint64_t i=0, j=0, k=0;
	uint64_t canopyIdx =0;
  uint64_t cntSoil =0;
	TYPE outSoil =0;
	TYPE outCanopy =0;
	TYPE leftEvap =0;
  #pragma omp parallel for private(i, j, k, cntSoil, outSoil, outCanopy, canopyIdx, leftEvap)
  for(i=0; i<numElements; i++) {
		cntSoil = i % 8; 
		outSoil =0;
		for (k=0; k<cntSoil; k++) {
			for (j=0; j<numSoilLayers; j++) {
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.523) / cntSoil;	
			}
		}
    outCanopy=0;
		for (canopyIdx=*(canopy_Reg_Set_Idx+i); canopyIdx < (*(canopy_Reg_Set_Idx+i+1)); canopyIdx++) {
			outCanopy += *(canopy_Reg_Set+canopyIdx) * 0.2375;
		}
		leftEvap = *(frac_Evap_Rand+(*(ar_Evap_Idx+i))) * 0.2589;
		*(out_EvapOut+i) = 	outSoil * outCanopy * leftEvap;
	}
}

void __attribute__ ((noinline))	funAtmosCalc(TYPE *out_AtmosEffect, uint64_t numElements,  
                TYPE *soil_Reg_Reuse, TYPE *atmos_Reg, uint64_t numAtmosValues)
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
				outSoil += (*(soil_Reg_Reuse+i+j)) * (0.5987); 
		}
		for (k=0; k<numAtmosValues; k++) {
				outAtmos += (*(atmos_Reg+i+k)) * (0.6521); 
		}
		*(out_AtmosEffect+i) = 	outSoil * outAtmos ;
	}
}	

int parseCommandLine(const int argc, char * const argv[], char *str_run_option)
{
	int opt;
  char *farMemData = NULL;
  if (argc <  2) {
        fprintf(stderr, "Usage: %s [-h] [-f reg|rand|both|regind|randind|bothind]\n", argv[0]);
  }
  // ':' after 'f' means it requires an argument
  while ((opt = getopt(argc, argv, "hf:")) != -1) {
    switch (opt) {
      case 'h':
          printf("Usage: %s [-h] [-f reg|rand|both|regind|randind|bothind]\n", argv[0]);
          break;
      case 'f':
          farMemData = optarg; // optarg stores the option's value
          printf("Far memory object specified access pattern is : %s \n", farMemData);
          break;
      default:
          fprintf(stderr, "Unknown option.\n");
          exit(1);
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
	} else if (strcmp(farMemData, "regind") == 0) {
    printf("Far memory object has regular access and its indices \n");
    farMemOption=4;
    sprintf(str_run_option, "RUNTIME reg-ind Far");
  } else if (strcmp(farMemData, "randind") == 0) {
    printf("Far memory object has random access and its indices \n");
    farMemOption=5;
    sprintf(str_run_option, "RUNTIME rand-ind Far");
  } else if (strcmp(farMemData, "bothind") == 0) {
    printf("Far memory object has regular and random access and its indices \n");
    farMemOption=6;
    sprintf(str_run_option, "RUNTIME both-ind Far");
  } else {
    printf("Usage: %s [-h] [-f reg|rand|both|regind|randind|bothind]\n", argv[0]);
    return 1;
  }
	return farMemOption; 
}

int main(int argc, char *argv[]) 
{
  struct timespec start, finish;            
	char *str_log=(char *) malloc(500*sizeof(char)); 
	char *str_run_option=(char *) malloc(500*sizeof(char)); 
  int farMemOption=0; 
	farMemOption = parseCommandLine(argc, argv, str_run_option) ;
  int total ;
  total = omp_get_max_threads();
  std::cout << "number of threads " <<  total << "\n";

	int numa_node = -1;
  numa_node = numa_preferred();
  int local_numa = numa_preferred();
  int far_numa = (local_numa == 0) ? 1 :0; 
	// For machines with single NUMA domain
	if (numa_node == -1) {
		local_numa =0;
		far_numa = 0;
	}
	printf("NUMA preferred %d local %d far %d \n", numa_node, local_numa, far_numa);

 	/* Regular Random - using random number of adjacent elements */	
	TYPE *veg_Reg_Set ;
	TYPE *root_Reg_Set ;

 	/* Random */	
	TYPE *sm_Rand ;
	TYPE *prec_Rand; // = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	
  /* Regular Random Indices */	
	uint64_t *veg_Reg_Set_Idx ; //= (uint64_t *)malloc (( numLat*numLon)*sizeof(uint64_t));
	uint64_t *root_Reg_Set_Idx; // = (uint64_t *)malloc (( numLat*numLon)*sizeof(uint64_t));
 	
  /* Random Indices */	
	uint64_t *sm_Rand_Idx; // = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));
	uint64_t *prec_Rand_Idx; // = (uint64_t *)malloc ((numLat*numLon)*sizeof(uint64_t));

	  veg_Reg_Set_Idx = (uint64_t *) malloc (( numLat*numLon)*sizeof(uint64_t));
	  root_Reg_Set_Idx = (uint64_t *) malloc (( numLat*numLon)*sizeof(uint64_t));
	  sm_Rand_Idx = (uint64_t *) malloc ((numLat*numLon)*sizeof(uint64_t));
	  prec_Rand_Idx = (uint64_t *) malloc ((numLat*numLon)*sizeof(uint64_t));
	  sm_Rand = (TYPE *) malloc (( numLat*numLon)*sizeof(TYPE));
	  prec_Rand = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	  veg_Reg_Set = (TYPE *) malloc (( numLat*numLon*numVegBands)*sizeof(TYPE));
	  root_Reg_Set = (TYPE *) malloc (( numLat*numLon*numRootLayers)*sizeof(TYPE));


 	/* Regular with reuse */	
	TYPE *soil_Reg_Reuse = (TYPE *) malloc (( numLat*numLon*numSoilLayers)*sizeof(TYPE));

  //TYPE *out_SurfMoist = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
	TYPE *out_PrecLeft = (TYPE *)malloc (( numLat*numLon)*sizeof(TYPE));
  
  #ifdef DEBUG 
    printf("start init_elem \n");
	#endif
  init_elem(veg_Reg_Set, numLat*numLon*numVegBands, 0.35);
  get_mempolicy(&numa_node, NULL, 0, (void*)veg_Reg_Set, MPOL_F_NODE | MPOL_F_ADDR);
	printf("NUMA veg_Reg_Set %d ", numa_node);
	init_elem(root_Reg_Set, numLat*numLon*numRootLayers, 0.678);
  get_mempolicy(&numa_node, NULL, 0, (void*)root_Reg_Set, MPOL_F_NODE | MPOL_F_ADDR);
	printf("root_Reg_Set %d ", numa_node);
	init_elem(sm_Rand, numLat*numLon, 0.375);
  get_mempolicy(&numa_node, NULL, 0, (void*)sm_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("sm_Rand %d ", numa_node);
	init_elem(prec_Rand, numLat*numLon, 0.345);
  get_mempolicy(&numa_node, NULL, 0, (void*)prec_Rand, MPOL_F_NODE | MPOL_F_ADDR);
	printf("prec_Rand %d \n", numa_node);

	init_elem(soil_Reg_Reuse, numLat*numLon*numSoilLayers, 0.355);
  get_mempolicy(&numa_node, NULL, 0, (void*)soil_Reg_Reuse, MPOL_F_NODE | MPOL_F_ADDR);
	printf("MALLOC soil_Reg_Reuse %d ", numa_node);
	//init_elem(atmos_Reg, numLat*numLon*numAtmosValues, 0.365);
  //get_mempolicy(&numa_node, NULL, 0, (void*)atmos_Reg, MPOL_F_NODE | MPOL_F_ADDR);
	//printf("atmos_Reg %d ", numa_node);

	
  #ifdef DEBUG 
    printf("start init_indices \n");
  #endif
  init_indices_Reg_Set(veg_Reg_Set_Idx , ( numLat*numLon), numVegBands);
  get_mempolicy(&numa_node, NULL, 0, (void*)veg_Reg_Set_Idx, MPOL_F_NODE | MPOL_F_ADDR);
	printf("veg_Reg_Set_Idx %d \n", numa_node);

	clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Random(sm_Rand_Idx, numLat*numLon);
	clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)sm_Rand_Idx, MPOL_F_NODE | MPOL_F_ADDR);
	printf("MALLOC sm_Rand_Idx %d ", numa_node);
	
  clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Random(prec_Rand_Idx, numLat*numLon);
  /* copy random in reverse order if needed
  #pragma omp parallel for
    for (uint64_t j=0; j<(numLat*numLon); j++) {
      *(prec_Rand_Idx+j) = *(sm_Rand_Idx+ (numLat*numLon)-1 -j);
	} */
  clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)prec_Rand_Idx, MPOL_F_NODE | MPOL_F_ADDR);
	printf("prec_Rand_Idx %d ", numa_node);

	clock_gettime(CLOCK_REALTIME, &start); 
	init_indices_Reg_Set(root_Reg_Set_Idx,  ( numLat*numLon), numRootLayers); 
	clock_gettime(CLOCK_REALTIME, &finish); 
  #ifdef DEBUG 
	  sprintf(str_log, "init reg rand indices time");  
	  print_time(str_log, start, finish);
  #endif
  get_mempolicy(&numa_node, NULL, 0, (void*)root_Reg_Set_Idx, MPOL_F_NODE | MPOL_F_ADDR);
	printf("root_Reg_Set_Idx %d \n", numa_node);
  
  //*out_SurfMoist = 0; 
	*out_PrecLeft = 0; 
  //get_mempolicy(&numa_node, NULL, 0, (void*)out_SurfMoist, MPOL_F_NODE | MPOL_F_ADDR);
	//printf("MALLOC out_SurfMoist %d ", numa_node);
  get_mempolicy(&numa_node, NULL, 0, (void*)out_PrecLeft, MPOL_F_NODE | MPOL_F_ADDR);
	printf("out_PrecLeft %d \n", numa_node);

	printf("Running function loop for %d iterations \n", numLoops); 

	clock_gettime(CLOCK_REALTIME, &start); 
	for ( int loopCnt=0; loopCnt <numLoops; loopCnt++) {
	  //funSMCalc(out_SurfMoist, (numLat*numLon), root_Reg_Set, root_Reg_Set_Idx, soil_Reg_Reuse, 
    //       veg_Reg_Set, veg_Reg_Set_Idx, sm_Rand, sm_Rand_Idx);  
		funPrecCalc(out_PrecLeft, (numLat*numLon), root_Reg_Set, root_Reg_Set_Idx, soil_Reg_Reuse,
               prec_Rand, prec_Rand_Idx);
	}
	clock_gettime(CLOCK_REALTIME, &finish); 

	//printf("sm %f %f \n", *(out_SurfMoist+1), *(out_SurfMoist+((numLat*numLon)-1)));
	printf("prec %f %f ", *(out_PrecLeft+1), *(out_PrecLeft+((numLat*numLon)-1)));
	
  print_time(str_run_option, start, finish);
  return 0;
}
