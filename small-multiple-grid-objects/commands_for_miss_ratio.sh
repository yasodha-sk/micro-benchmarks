OMP_DISPLAY_ENV=true OMP_PROC_BIND="close" OMP_PLACES="{0}:16:2" OMP_NUM_THREADS=16 perf record -o perf_k_0.perf -e LLC-load-misses:u -e LLC-loads:u  ./exe-grid
  perf script -i perf_k_0.perf > grid-llc-script.txt
  grep LLC-loads grid-llc-script.txt > grid-llc-script-LLC-loads.txt
  grep LLC-load-misses grid-llc-script.txt > grid-llc-script-LLC-miss.txt
  grep fun grid-llc-script-LLC-miss.txt > grid-llc-script-LLC-miss-func.txt
  grep fun grid-llc-script-LLC-loads.txt > grid-llc-script-LLC-loads-func.txt
  awk '{print $6}' grid-llc-script-LLC-loads-func.txt | sort | uniq > grid-llc-script-LLC-loads-func-ip.txt
  awk '{print $6}' grid-llc-script-LLC-miss-func.txt | sort | uniq > grid-llc-script-LLC-miss-func-ip.txt
