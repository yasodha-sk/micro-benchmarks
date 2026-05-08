set -x
repeatCnt=10
set -x
cur_date=$(date +"%d-%m")
threads=16


for i in $( seq 1 $repeatCnt )
do
#OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:${threads}:1"	OMP_NUM_THREADS=${threads} ./exe-arr 
OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="cores"	OMP_NUM_THREADS=${threads} ./exe-arr 
done
