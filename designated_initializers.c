/**
 * Designated Initializers from GCC's extension.
 * If compile with -pedantic, it will not allow compilation to pass.
 * It will shout out error of
 *
 * "ISO C forbids specifying range of elements to initialize [-Werror=pedantic]"
 *
 * Compile it with
 *
 * gcc -std=gnu99 -Wall -Wextra -Werror -Wno-override-init designated_initializers.c
 */
#include<assert.h>

#define ARRAY_SIZE 5

int main(void)
{
	// whole range of array assigned to a single value
	{
		int array[ARRAY_SIZE] = {
			[0 ... ARRAY_SIZE-1] = 0
		};

		for (int i=0; i<ARRAY_SIZE; ++i)
		{
			assert(array[i] == 0);
		}
	}

	// same as above but also assign individuals with different value
	// we've seen this use case in arch/arc/kernel/sys.c of linux kernel
	// in order to override, one needs to compile with disable of override-init via
	// -Wno-override-init. Linux kernel also disables this error.
	{
		int array[ARRAY_SIZE] = {
			[0 ... ARRAY_SIZE-1] = 0,
			[0] = 1,
			[1] = 2,
			[2] = 3,
			[3] = 4,
			[4] = 5
		};

		for (int i=0; i<ARRAY_SIZE; ++i)
		{
			assert(array[i] == (i+1));
		}
	}
	
	return 0;
}
