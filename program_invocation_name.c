/**
 * As seen it is used by kmod to dynamically call specific operation function
 * e.g. do_lsmod, do_rmmod, etc if user execute different symlinked files
 * which all link back to the same executable binary kmod.
 *
 * This is a technique for GNU toolchain which means GCC, but not Clang.
 */
// these lines need to be before other main header files
// so for us to avoid defining `extern char* program_invocation_...` again
#define _GNU_SOURCE
#include <errno.h>

#include <stdio.h>

int main(void)
{
	printf("program_invocation_name = %s\n", program_invocation_name);
	printf("program_invocation_short_name = %s\n", program_invocation_short_name);
	return 0;
}
