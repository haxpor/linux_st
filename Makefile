.PHONY: all clean

MK=make
CC=gcc-10
# closely align with standard version used by kernel
# NOTE: on gcc 9+ onwards don't have gnu98 (which is main for compiling kernel code)
COMMON_C_FLAGS=-std=gnu99 -Wall -Wextra -O2 -g

# declare all dependent targets here
# each line should not be longer than 80 characters column, no matter how many
# dependent targets there will be for each line
TARGET_DEPS := unistd.out \
	static_inline_func_ptrchk.out \
	designated_initializers.out \
	signal.out \
	compile_time_type_checking.out \
	execve.dir                         # targets that live in separte dircectory suffixes with .dir

all: $(TARGET_DEPS)
	@echo "All done"

# ---- target listing for individual source files should be listed here onwards ----

# list of targets without file extensions
# have .out for ease in cleaning out later
unistd.out: unistd.c
	$(CC) $(COMMON_C_FLAGS) $^ -o $@

# Testing the technique of defining 'static inline' function pointer that just
# return function pointer that just received in via macro.
# This can help checking signature of input function at compile time.
#
# NOTE: make the warning of incompatible pointer type as error
static_inline_func_ptrchk.out: static_inline_func_ptrchk.c
	$(CC) $(COMMON_C_FLAGS) -Werror=incompatible-pointer-types $^ -o $@

# NOTE: we need to allow overriden in initialization
designated_initializers.out: designated_initializers.c
	$(CC) $(COMMON_C_FLAGS) -Wno-override-init $^ -o $@

signal.out: signal.c
	$(CC) $(COMMON_C_FLAGS) $^ -o $@

compile_time_type_checking.out: compile_time_type_checking.c
	$(CC) $(COMMON_C_FLAGS) $^ -o $@

# ---- target listing for program living in separate directory  should be listed here onwards ----

# target depends on all files inside the directory
execve.dir: $(shell find $(basename $@) -type f)
	$(MK) -C $(basename $@) -f Makefile

clean:
	$(MK) -C execve -f Makefile clean
	rm -rf *.o *.out
