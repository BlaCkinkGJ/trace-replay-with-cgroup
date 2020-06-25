#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;
	int status;

	if ((pid = fork()) < 0) { /* fork failed */
	}
}