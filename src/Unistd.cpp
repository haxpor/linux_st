// As we didn't have control over function signature of execv() which accept 
// char *const in constrast that we have 'const char'. This produce compilation warning that
// we could silence it with the following #pragma line. Note that we can do it at complie flags too via
// 'gcc -Wno-write-strings ...'.
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <iostream>
#include <unistd.h>

int main()
{
    // getpid()
    std::cout << "PID of this program is " << getpid() << std::endl;

    // execv()
    char* const args[] = {"-la", (char* const)nullptr};
    execv("ls", args);
    return 0;
}
