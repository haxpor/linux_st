/**
 * Small check to validate shorted circuit.
 * Inspired from https://lore.kernel.org/linux-hardening/20211015050345.GA1161918@embeddedor/.
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
	int val;
} dummy_struct;

int main(void)
{
	dummy_struct* ptr = malloc(sizeof(dummy_struct));
	ptr->val = 1;

	// case 1 - check for positive
	// ptr->val won't be evaluated as ptr is NULL (shorted circuit)
	if (ptr && ptr->val == 1)
		assert(true && "This should be called\n");

	free(ptr);

	ptr = NULL;

	// case 2 - check for negative
	// ptr is NULL, then ptr->val won't be evaluated (shorted circuit)
	if (!ptr || ptr->val == 0)
		assert(true && "This should be called\n");

	return 0;
}
