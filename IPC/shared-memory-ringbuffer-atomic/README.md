# Shared Memory Mapped with Ring Buffer - enhanced with atomic

This demonstrates inter-process communication through shared memory map between writer and reader process.
A ring buffer implemented using atomic variable.

For testing, launch a single `writer`, and multiple `reader` to see the rate of production, and consumption from the two sides.

You can also build in behcmark mode via `make bench`, in which now it `reader` can accept output time series file.

Ex. `./reader ts1.txt`

# Plot chart of cache latency with R

Execute `Rscript plotchart.R <input-ts-file> <output-image-file>`
