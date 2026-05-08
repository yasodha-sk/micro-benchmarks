set -x
repeatCnt=5
set -x
cur_date=$(date +"%d-%m")
threads=32 


for i in $( seq 1 $repeatCnt )
       do
 perf record -o perf_k_0.perf -e mem_inst_retired.all_loads:u -e mem_inst_retired.any:u -e LLC-load-misses:u ./arr_op
perf report -i perf_k_0.perf -v > llc_miss_loads_any_user_${i}-stride512.txt
done
