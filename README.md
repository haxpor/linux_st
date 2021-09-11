# How to Build

* `make` or `make all` - build all source files either from individual source files, or source files living in separate directories
* `make <source-file-without-extension>` - build individual source file
* `make <dir-name>.dir` - build individual program living in `dir-name` e.g. `make execve.dir`
* `make clean` - clean all built artifacts for from both individual source files, and source files living in separate directories

# License

GPL-2.0, Wasin Thonkaew

Previously changed from MIT, to GPL-2.0 as most of the ideas of concepts are
inspired from Linux Kernel source code. Source code can be similar to what has
been there in the source code of Linux Kernel, almost it's not pure copy, but
adapted or modified in some ways. Thus to keep it compatible and avoid
breaking the license, this repository opts in to use GPL-2.0 the same.
