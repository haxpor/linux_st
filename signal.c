/**
 * Test using signal() function
 *
 * Compile with
 * gcc -std=gnu99 -Wall -Wextra -pedantic signal.c
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void signal_int_handler(int s)
{
	printf("signal_int_handler() called\n");
	exit(0);
}

int main(void)
{
	signal(SIGINT, signal_int_handler);
	while (1) {}
	return 0;
}
