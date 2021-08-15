.PHONY: all clean

CC=gcc-9
# closely align with standard version used by kernel
# NOTE: on gcc 9+ onwards don't have gnu98 (which is main for compiling kernel code)
COMMON_C_FLAGS=-std=gnu99 -Wall -g

# declare all dependent targets here
# each line should not be longer than 80 characters column, no matter how many
# dependent targets there will be for each line
TARGET_DEPS := Unistd.out StaticInlineFunctionPointerCheck.out

all: $(TARGET_DEPS)

# list of targets without file extensions
# have .out for ease in cleaning out later
Unistd.out: Unistd.c
	$(CC) $(COMMON_C_FLAGS) $^ -o $@

# Testing the technique of defining 'static inline' function pointer that just
# return function pointer that just received in via macro.
# This can help checking signature of input function at compile time.
#
# NOTE: make the warning of incompatible pointer type as error
StaticInlineFunctionPointerCheck.out: StaticInlineFunctionPointerCheck.c
	$(CC) $(COMMON_C_FLAGS) -Werror=incompatible-pointer-types $^ -o $@

clean:
	rm -rf *.o *.out
