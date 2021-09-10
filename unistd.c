/**
 * Demonstrate calling some APIs inside unistd.h header (POSIX Operating System APIs level)
 */

// As we didn't have control over function signature of execv() which accept 
// char *const in constrast that we have 'const char'. This produce compilation warning that
// we could silence it with the following #pragma line. Note that we can do it at complie flags too via
// 'gcc -Wno-write-strings ...'.
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <stdio.h>
#include <unistd.h>

int main()
{
    // getpid()
	// pid_t is type-defed to int
    printf("PID of this program is %u\n", getpid());

    // execv()
    char* const args[] = {"-la", (char* const)NULL};
    execv("ls", args);
    return 0;
}
