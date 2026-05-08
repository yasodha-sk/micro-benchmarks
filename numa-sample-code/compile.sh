export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yasodhan/benchmark/numactl/.libs/
gcc -v alloc_on_node.c -I/home/yasodhan/benchmark/numactl/ -L /home/yasodhan/benchmark/numactl/.libs/ -o alloc-node -lnuma
gcc distance.c -I/home/yasodhan/benchmark/numactl/ -L /home/yasodhan/benchmark/numactl/.libs/ -o dist-node -lnuma
