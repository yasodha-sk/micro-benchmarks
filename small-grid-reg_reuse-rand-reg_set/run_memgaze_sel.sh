set +x
smpl_period=1000000
buffer_size=16384
cwd='/home/yasodhan/Projects/Benchmarks/micro-benchmarks/small-grid-reg_reuse-rand-reg_set'
mg_dir='/home/yasodhan/Projects/memgaze'
dir_name="memgaze-ub1-sel" 

rm -rf $cwd/$dir_name
mkdir $cwd/$dir_name
cd $cwd/$dir_name
cp $cwd/exe-grid $cwd/$dir_name
# $mg_dir/install/bin/memgaze-inst ./exe-grid -l 1 -s 0 -f 0 -o $cwd/$dir_name

export DYNINSTAPI_RT_LIB=/home/yasodhan/Projects/memgaze/install/bin/../../xlib/lib/dyninst-12.1.0/lib/libdyninstAPI_RT.so
/home/yasodhan/Projects/memgaze/install/bin/../libexec/bin/memgaze-instrumentor --bin_path $cwd/$dir_name/exe-grid  --load_class 1 --inst_loads=1 --inst_stores=0 --inst_strided=1     --inst_indirect=1 --inst_frame=0  --func_list=1000 --func_list=1770 --func_list=2970 --func_list=29e0 --func_list=2a70 --func_list=2b00 --func_list=2b60 --func_list=2fe0 --func_list=3380 --func_list=35f0 --func_list=3710 --func_list=3780 --func_list=40b0 --func_list=4d70 --func_list=4d90 --func_list=4db0 --func_list=4ec0 --func_list=4ed0 --func_list=4f40 --func_list=5540 --lcFile=$cwd/$dir_name/XSBench-sel-memgaze.binanlys --outBinName=$cwd/$dir_name/XSBench-sel-memgaze &> $cwd/$dir_name/XSBench-sel-memgaze.binanlys.log
/home/suri836/Projects/memgaze_gh/memgaze/install/bin/../libexec/memgaze-inst-cat $cwd/$dir_name/exe-grid-sel-memgaze.binanlys.log $cwd/$dir_name/exe-grid-sel-memgaze.binanlys $cwd/$dir_name/exe-grid-sel-memgaze
/home/yasodhan/Projects/memgaze/install/bin/../../xlib/lib/hpctoolkit-2023.08.stable/bin/hpcstruct $cwd/$dir_name/exe-grid-sel-memgaze -o $cwd/$dir_name/exe-grid-sel-memgaze.hpcstruct
mv $cwd/$dir_name/exe-grid-sel-memgaze.binanlys $cwd/$dir_name/exe-grid-sel-memgaze.binanlys_old
mv $cwd/$dir_name/exe-grid-sel-memgaze.binanlys_Fixed $cwd/$dir_name/exe-grid-sel-memgaze.binanlys
objdump -dC exe-grid-sel-memgaze > obj_exe-grid

#00000000000029e0 <run_event_based_simulation>:
#0000000000002a70 <run_history_based_simulation>:
#0000000000002b00 <grid_search>:
#0000000000002b60 <grid_search_nuclide>:
#0000000000002bd0 <calculate_micro_xs>:
#0000000000002de0 <calculate_macro_xs>:
#0000000000002fe0 <run_event_based_simulation_optimization_1._omp_fn.9>:
#00000000000031f0 <LCG_random_double>:
#0000000000003230 <pick_mat>:
#0000000000003380 <run_event_based_simulation._omp_fn.0>:
#00000000000035f0 <run_event_based_simulation_optimization_1._omp_fn.8>:
#0000000000003710 <fast_forward_LCG>:
#0000000000003780 <run_history_based_simulation._omp_fn.1>:
#0000000000003a70 <quickSort_parallel_internal_i_d>:
#0000000000003c90 <quickSort_parallel_internal_i_d._omp_fn.3>:
#0000000000003cb0 <quickSort_parallel_internal_i_d._omp_fn.2>:
#0000000000003cd0 <quickSort_parallel_i_d._omp_fn.4>:
#0000000000003d20 <quickSort_parallel_i_d>:

cd $cwd/$dir_name
objdump -ldC exe-grid > obj_exe-grid 
objdump -dC exe-grid-memgaze > obj_exe-grid_inst


cd $cwd/$dir_name
export OMP_PROC_BIND="close" 
export OMP_PLACES="{0:1}:8:2" 
export OMP_NUM_THREADS=4
$mg_dir/install/bin/memgaze-run -p $smpl_period -b $buffer_size -- ./exe-grid-memgaze 
$mg_dir/install/bin/memgaze-xtrace ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period
$mg_dir/install/bin/memgaze-analyze ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/ -o ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/ > ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/anlys_log
$mg_dir/install/bin/memgaze-analyze-loc ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/ -s 1 -o ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/ > ./exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/loc_log

python3 $mg_dir/mem-anlys/loc-anlys/aux_scripts/getRUDInsnMap.py --i  $cwd/$dir_name/exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/spatial.txt_insn --o $cwd/$dir_name/exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/spatial_insn --app grid-ub1 &> $cwd/$dir_name/exe-grid-memgaze-trace-b$buffer_size-p$smpl_period/log_mapping
