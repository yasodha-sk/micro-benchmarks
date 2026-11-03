set +x
smpl_period=1000000
buffer_size=16384
cwd='/home/yasodhan/Projects/Benchmarks/micro-benchmarks/small-grid-reg_reuse-rand-reg_set'
mg_dir='/home/yasodhan/Projects/memgaze'
dir_name="memgaze-ub1" 

rm -rf $cwd/$dir_name
mkdir $cwd/$dir_name
cd $cwd/$dir_name
cp $cwd/exe-grid $cwd/$dir_name
$mg_dir/install/bin/memgaze-inst ./exe-grid -l 1 -s 0 -f 0 -o $cwd/$dir_name
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
