
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

#define MAX_LAYERS 3
#define MONTHS_PER_YEAR 12

typedef struct {
	 double Ds;                        /**< fraction of maximum subsurface flow
						                                             rate */
	 double Dsmax;                     /**< maximum subsurface flow rate
							                                         (mm/day) */
	 double Wcr[MAX_LAYERS];           /**< critical moisture level for soil
								                                         layer, evaporation is no longer
													                                          affected moisture stress in the
																		                                           soil (mm) */
   double Ws;                        /**< fraction of maximum soil moisture */
	 double annual_prec;               /**< annual average precipitation (mm) */
	 double avg_temp;                  /**< average soil temperature (C) */
	 double avgJulyAirTemp;            /**< Average July air temperature (C) */
	 double soil_density[MAX_LAYERS];  /**< soil particle density (kg/m^3) */
	 double soil_dens_min[MAX_LAYERS]; /**< particle density of mineral soil (kg/m^3) */
	 double soil_dens_org[MAX_LAYERS]; /**< particle density of organic soil (kg/m^3) */
	 double *Tfactor;                  /**< Change in temperature due to elevation (C) in each snow elevation band */
	 bool *AboveTreeLine;             /**< Flag to indicate if band is above the treeline */
	 double elevation;                 /**< grid cell elevation (m) */
	 double lat;                       /**< grid cell central latitude */
   double lng;                       /**< grid cell central longitude */
   double cell_area;                 /**< Area of grid cell (m^2) */
} structSoil; 

typedef struct {
 double albedo[MONTHS_PER_YEAR];   /**< climatological vegetation albedo (fraction) */
 double *CanopLayerBnd;  /**< Upper boundary of each canopy layer, expressed as fraction of total LAI */
  double Cv;              /**< fraction of vegetation coverage */
  double displacement[MONTHS_PER_YEAR]; /**< climatological vegetation displacement height (m) */
   double fcanopy[MONTHS_PER_YEAR]; /**< climatological fractional area covered by plant canopy (fraction) */
   double fetch;           /**< Average fetch length for each vegetation class. */
     double root[MAX_LAYERS]; /**< percent of roots in each soil layer (fraction) */
  double roughness[MONTHS_PER_YEAR]; /**< climatological vegetation roughness length (m) */
      double sigma_slope;     /**< Std. deviation of terrain slope for each vegetation class */
   int veg_class;          /**< vegetation class id number */
   size_t vegetat_type_num; /**< number of vegetation types in the grid cell */
    double *zone_depth;     /**< depth of root zone */
 double *zone_fract;     /**< fraction of roots within root zone */
} structVegCon;

