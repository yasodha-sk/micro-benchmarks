
   OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=0 ./exe-grid -f both
   OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=0 ./exe-grid -f regind
   OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=0 ./exe-grid

