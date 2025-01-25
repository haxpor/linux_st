# Shared Memory Mapped with Ring Buffer

This demonstrates inter-process communication through shared memory map between writer and reader process.
A ring buffer implemented using `pthread_rwlock_t` aka. read-write lock for shared access, and exclusive write.

For testing, launch a single `writer`, and multiple `reader` to see the rate of production, and consumption from the two sides.
