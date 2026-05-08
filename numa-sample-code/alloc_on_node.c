/* Test numa_distance */
#include <numa.h>
#include <numaif.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int maxnode, a, b, got_nodes = 0;
	int *node_to_use;
	if (numa_available() < 0) {
		printf("no numa support in kernel\n");
		exit(1);
	}
	maxnode = numa_max_node();
	node_to_use = (int *)malloc(maxnode * sizeof(int));
	for (a = 0; a <= maxnode; a++) {
		if (numa_bitmask_isbitset(numa_nodes_ptr, a)){
			node_to_use[got_nodes++] = a;
		}
	}

	for (a = 0; a < got_nodes; a++){
		printf("%03d: ", node_to_use[a]);
		if (numa_distance(node_to_use[a], node_to_use[a]) != 10) {
			printf("%d: self distance is not 10 (%d)\n",
			       node_to_use[a], numa_distance(node_to_use[a],node_to_use[a]));
			exit(1);
		}
		for (b = 0; b < got_nodes; b++) {
			int d1 = numa_distance(node_to_use[a], node_to_use[b]);
			int d2 = numa_distance(node_to_use[b], node_to_use[a]);
			printf("%03d ", d1);
			if (d1 != d2) {
				printf("\n(%d,%d)->(%d,%d) wrong!\n",node_to_use[a],node_to_use[b],d1,d2);
				exit(1);
			}
		}
		printf("\n");
	}
	int numa_node = -1;
	int * node_0;     
	int * node_1;     
	int * node_2;     
	int * node_3;     
	for (int i =0; i<10; i++){
	node_0     = (int *) numa_alloc_onnode( 1024 *1024*  sizeof(int),0);
	*node_0 = 2;
        if(get_mempolicy(&numa_node, NULL, 0, (void*)node_0, MPOL_F_NODE | MPOL_F_ADDR) < 0)
		printf(" get_mempolicy failed\n");
        printf("numa_node node_0 %d %p \n", numa_node, node_0);
	*node_0 = 2;
	numa_node = -1;
	node_1     = (int *) numa_alloc_onnode( 1024* 1024 *1024*  sizeof(int),1);
	*node_1 = 3;
        if (get_mempolicy(&numa_node, NULL, 0, (void*)node_1, MPOL_F_NODE | MPOL_F_ADDR) <0 )
		printf(" get_mempolicy failed\n");
        printf("numa_node node_1 %d %p \n", numa_node, node_1);
	*node_1 = 3;
	//node_2     = (int *) numa_alloc_onnode( 1024 *1024*  sizeof(int),1);
	node_2     = (int *) malloc( 1024 *1024*  sizeof(int));
	*node_2 = 2;
	numa_node = -1;
        if(get_mempolicy(&numa_node, NULL, 0, (void*)node_2, MPOL_F_NODE | MPOL_F_ADDR) <0 )
		printf(" get_mempolicy failed\n");
        printf("numa_node node_2 %d %p \n", numa_node, node_2);
	*node_2 = 2;
	
	//node_3     = (int *) numa_alloc_onnode( 1024 *1024*  sizeof(int),1);
	node_3     = (int *) malloc( 1024 *1024*  sizeof(int));
	*node_3 = 2;
	numa_node = -1;
        if(get_mempolicy(&numa_node, NULL, 0, (void*)node_3, MPOL_F_NODE | MPOL_F_ADDR) <0 )
		printf(" get_mempolicy failed\n");
        printf("numa_node node_3 %d %p \n", numa_node, node_3);
	*node_3 = 2;
	}
	return 0;
}
