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

export DYNINSTAPI_RT_LIB=$mg_dir/install/bin/../../xlib/lib/dyninst-12.1.0/lib/libdyninstAPI_RT.so
/home/yasodhan/Projects/memgaze/install/bin/../libexec/bin/memgaze-instrumentor --bin_path $cwd/$dir_name/exe-grid  --load_class 1 --inst_loads=1 --inst_stores=0 --inst_strided=1 --inst_indirect=1 --inst_frame=0 --func_list=2dd0 --func_list=3060 --func_list=3800 --func_list=3ae0 --func_list=3b60 --lcFile=$cwd/$dir_name/exe-grid-sel-memgaze.binanlys --outBinName=$cwd/$dir_name/exe-grid-sel-memgaze &> $cwd/$dir_name/exe-grid-sel-memgaze.binanlys.log
$mg_dir/install/bin/../libexec/memgaze-inst-cat $cwd/$dir_name/exe-grid-sel-memgaze.binanlys.log $cwd/$dir_name/exe-grid-sel-memgaze.binanlys $cwd/$dir_name/exe-grid-sel-memgaze
$mg_dir/install/bin/../../xlib/lib/hpctoolkit-2023.08.stable/bin/hpcstruct $cwd/$dir_name/exe-grid-sel-memgaze -o $cwd/$dir_name/exe-grid-sel-memgaze.hpcstruct
mv $cwd/$dir_name/exe-grid-sel-memgaze.binanlys $cwd/$dir_name/exe-grid-sel-memgaze.binanlys_old
mv $cwd/$dir_name/exe-grid-sel-memgaze.binanlys_Fixed $cwd/$dir_name/exe-grid-sel-memgaze.binanlys

#
#0000000000002dd0 <funSMCalc(double*,
#0000000000003060 <funPrecCalc(double*,
#0000000000003300 <funEvapCalc(double*,
#00000000000035b0 <funAtmosCalc(double*,
#0000000000003800 <funSMCalc(double*,
#0000000000003ae0 <funSMCalc(double*,
#0000000000003b60 <funPrecCalc(double*,
#0000000000003bd0 <funEvapCalc(double*,
#0000000000003c40 <funAtmosCalc(double*,
#

cd $cwd/$dir_name
objdump -ldC exe-grid > obj_exe-grid 
objdump -dC exe-grid-sel-memgaze > obj_exe-grid_inst

if [ 1 -eq 1 ]; then

cd $cwd/$dir_name
export OMP_PROC_BIND="close" 
export OMP_PLACES="{0:1}:8:2" 
export OMP_NUM_THREADS=4
$mg_dir/install/bin/memgaze-run -p $smpl_period -b $buffer_size -- ./exe-grid-sel-memgaze 
$mg_dir/install/bin/memgaze-xtrace ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period
$mg_dir/install/bin/memgaze-analyze ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/ -o ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/ > ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/anlys_log
$mg_dir/install/bin/memgaze-analyze-loc ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/ -s 1 -o ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/ > ./exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/loc_log

python3 $mg_dir/mem-anlys/loc-anlys/aux_scripts/getRUDInsnMap.py --i  $cwd/$dir_name/exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/spatial.txt_insn --o $cwd/$dir_name/exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/spatial_insn --app grid-ub1 &> $cwd/$dir_name/exe-grid-sel-memgaze-trace-b$buffer_size-p$smpl_period/log_mapping

fi
