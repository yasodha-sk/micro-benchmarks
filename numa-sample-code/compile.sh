export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yasodhan/oldhomedir/benchmark/numactl/.libs/
gcc -v alloc_on_node.c -I/home/yasodhan/oldhomedir/benchmark/numactl/ -L /home/yasodhan/oldhomedir/benchmark/numactl/.libs/ -o alloc-node -lnuma
gcc distance.c -I/home/yasodhan/oldhomedir/benchmark/numactl/ -L /home/yasodhan/oldhomedir/benchmark/numactl/.libs/ -o dist-node -lnuma
gcc prefered.c -I/home/yasodhan/oldhomedir/benchmark/numactl/ -L /home/yasodhan/oldhomedir/benchmark/numactl/.libs/ -o pref-node -lnuma
