objdump -lDC exe-grid > obj_exe-grid
#OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=4 perf record -o perf_k_0.perf -e LLC-load-misses:u -e LLC-loads:u  -e mem_inst_retired.all_loads:u ./exe-grid
OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=4 perf record -o perf_k_0.perf -e mem_inst_retired.all_loads:pp -e mem_load_retired.l1_miss:pp -e mem_load_retired.l2_miss:pp -e mem_load_retired.l3_hit:pp -e mem_load_retired.l3_miss:pp ./exe-grid
#OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=4 perf record -o perf_k_0.perf -e LLC-load-misses:u -e LLC-loads:u  -e L1-dcache-loads:u -e mem_inst_retired.all_loads:u -e mem_load_retired.l3_miss:u ./exe-grid

  perf script -i perf_k_0.perf > grid-llc-script.txt
  grep mem_load_retired.l2_miss grid-llc-script.txt > grid-llc-script-LLC-loads.txt
  grep mem_load_retired.l3_miss grid-llc-script.txt > grid-llc-script-LLC-miss.txt
  grep mem_inst_retired.all_loads grid-llc-script.txt > grid-llc-script-all-loads.txt
  #grep L1-dcache-loads grid-llc-script.txt > grid-llc-script-all-loads.txt
  grep fun grid-llc-script-LLC-miss.txt > grid-llc-script-LLC-miss-func.txt
  grep fun grid-llc-script-LLC-loads.txt > grid-llc-script-LLC-loads-func.txt
  grep fun grid-llc-script-all-loads.txt > grid-llc-script-all-loads-func.txt
  awk '{print $6}' grid-llc-script-LLC-loads-func.txt | sort | uniq > grid-llc-script-LLC-loads-func-ip.txt
  awk '{print $6}' grid-llc-script-LLC-miss-func.txt | sort | uniq > grid-llc-script-LLC-miss-func-ip.txt
  awk '{print $6}' grid-llc-script-all-loads-func.txt | sort | uniq > grid-llc-script-all-loads-func-ip.txt
  python getPerfLoadMiss.py ./obj_exe-grid> grid-miss-rate.txt
