set -x
repeatCnt=5
set -x
cur_date=$(date +"%d-%m")
threads=16


for i in $( seq 1 $repeatCnt )
do
#OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="cores"	OMP_NUM_THREADS=${threads} ./exe-grid 
  OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=1 ./exe-grid
done

for i in $( seq 1 $repeatCnt )
do
#OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="cores"	OMP_NUM_THREADS=${threads} ./exe-grid 
  OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=0 ./exe-grid
done
