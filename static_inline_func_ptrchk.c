/*
 * We have seen module_init() / module_exit() macro which accepts input
 * of function defined at user site. Inside the macro, it makes use of
 * static inline definition of function that just return what users pass in.
 *
 * This is for purpose of checking if the type/signature of the input function
 * pointer is correct at compile time.
 */

#include <stdio.h>

typedef void (*func_sig_t)(void);

// the syntax is not 100% the same as seen in linux kernel code
// this is like a function type checking in C
#define FUNC_CHK(func)	\
	static inline __attribute__((unused)) void (*__func_chk##_##func)(void) = func


static void myCorrectFunc(void)
{
}

static int myWrongFunc(void)
{
	return 1;
}

int main(void)
{
	FUNC_CHK(myCorrectFunc);

	// uncomment this line to see the compile error
	//FUNC_CHK(myWrongFunc);
	return 0;
}
