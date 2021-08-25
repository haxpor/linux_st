#!/bin/bash
# A quick and simple bash script to work with execve.c without a need
# to create a C program.

echo "\$USER_DEFINED is $USER_DEFINE"

# hard-coded validate that USER_DEFINED should have value of "Dummy"
if [ "$USER_DEFINE" = "Dummy" ]; then
	echo "Passed"
else
	echo "Failed"
	return -1
fi
