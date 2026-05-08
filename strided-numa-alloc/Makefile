SOURCE = array_op.c 
CC = gcc
CFLAGS = -g -Wall -O3

.PHONY: all
all :  arr_op

arr_pnta_16: $(SOURCE)
	$(CC) $(CFLAGS) -DDO_PREFETCH_NTA -DPD_16 $(SOURCE) -o array_pnta_16 
arr_pnta_8: $(SOURCE)
	$(CC) $(CFLAGS) -DDO_PREFETCH_NTA -DPD_8 $(SOURCE) -o array_pnta_8 
arr_pnta_4: $(SOURCE)
	$(CC) $(CFLAGS) -DDO_PREFETCH_NTA -DPD_4 $(SOURCE) -o array_pnta_4 
arr_pnta_0: $(SOURCE)
	$(CC) $(CFLAGS) -DDO_PREFETCH_NTA_ADV0 $(SOURCE) -o array_pnta_0 
arr_pt_8: $(SOURCE)
	$(CC) $(CFLAGS) -DDO_PREFETCH_T0 $(SOURCE) -o array_pt_8  
arr_op: $(SOURCE) 
	$(CC) $(CFLAGS) $(SOURCE) -o arr_op


clean:
	rm ./array_pnta_16 
	rm ./array_pnta_8 
	rm ./array_pnta_4 
	rm ./array_pnta_0 
	rm ./array_pt_8 
	rm ./arr_op 
