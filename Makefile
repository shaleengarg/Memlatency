all: cache

cache: cache-tlb.c
	gcc -o CacheTlb cache-tlb.c -lm
