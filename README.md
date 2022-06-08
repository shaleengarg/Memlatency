# Memlatency

1.	Cache Line Size
Cacheline size was determined by timing the access times while increasing the stride of accessing an array. As soon as the stride gets past the Cacheline size, there is a bump in the access time. This size is captured and results are produced.

Here, the biggest hurdle was to prevent prefetching. To do so, I multiplied the stride by an additive and then computed the bit wise AND with the max length (To prevent segfault).

This way each access was a new one, and hence once the stride moved past the cacheline size, each new access increasingly became a cache-miss and hence we know our cacheline size. Stride jumped by two times each time. This is because continuous access lets the prefetchers prefetch all the contents; hence no bump in time.

Before doing all of this, just after allocating memory for the array access, I touched each page once to remove all minor-page faults. This removes noise from our calculations.

2.	Cache Size
My computer has three caches: L1, L2, L3.
I have used the same technique as the Cacheline size test. Here, we access a new cache line in each access. Each access is a new one, hence old cache entries can not be used. At some point, the access will not be able to fit in the cache, this is when my access will be catered by a slower cache; capture these bumps. These are your cache sizes.
I have defined one function which is designed to give all the cache sizes. It takes in approximate start size and end size for the cache and gives out the size of cache based on access time bumps.

3.	Memory Latency
To prevent any cache involvement for accesses, I have defined a linked list. Each node in this linked list is worth a page size(4098K). This makes the prefetches useless because they prefetch at a page-level.
An average of the total time of numerous accesses is taken and returned.

