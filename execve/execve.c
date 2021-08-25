/**
 * Prove-of-concept to set environment variable before executing a new binary
 * program from main process.
 *
 * This technique is used by valgrind to pass in LD_PRELOAD when execute its
 * children of tools e.g. memcheck in order for it to use its own defined malloc
 * function to track down memory.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
	char* argv[] = { NULL };
	char* envp[] = { "USER_DEFINE=Dummy", NULL };
	int ret = execve("./wrapped_sh.sh", argv, envp);
	if (ret == -1)
	{
		perror("execve");
		exit(EXIT_FAILURE);
	}
	return 0;
}
