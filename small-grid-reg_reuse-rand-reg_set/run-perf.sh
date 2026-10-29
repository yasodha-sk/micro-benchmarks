set -x
repeatCnt=2
set -x
cur_date=$(date +"%d-%m")
threads=16


for i in $( seq 1 $repeatCnt )
do
  OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 numactl --membind=0 perf stat -e mem_inst_retired.all_loads -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss -e mem_load_retired.l2_miss  -e mem_load_retired.l3_miss -e mem_load_l3_miss_retired.local_dram -e mem_load_l3_miss_retired.remote_dram ./exe-grid
done

for i in $( seq 1 $repeatCnt )
do
  OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{1}:16:2" OMP_NUM_THREADS=16 numactl --membind=1 perf stat -e mem_inst_retired.all_loads -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss -e mem_load_retired.l2_miss  -e mem_load_retired.l3_miss -e mem_load_l3_miss_retired.local_dram -e mem_load_l3_miss_retired.remote_dram ./exe-grid
done
