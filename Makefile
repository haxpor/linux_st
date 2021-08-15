.PHONY: all clean

CC=gcc-9
# closely align with standard version used by kernel
# NOTE: on gcc 9+ onwards don't have gnu98 (which is main for compiling kernel code)
COMMON_C_FLAGS=-std=gnu99 -Wall -Wextra -pedantic

all: Unistd.out

# list of targets without file extensions
# have .out for ease in cleaning out later
Unistd.out: Unistd.c
	$(CC) $(COMMON_C_FLAGS) $^ -o $@

clean:
	rm -rf *.o *.out
