#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "sync.h"

#define MAX_TASK (4)
#define MAX_COMMAND_SIZE (1024)
#define DEFAULT_WEIGHT (100)
#define PREFIX_CGOURP_NAME "tester.trace."

/***** FUNCTION DECLARATION PART *****/
int read_config_file(const char *file_name, int weight[],
		     char bench_file[MAX_TASK][FILENAME_MAX]);
int bench_exec_process(int weight, char bench_file[]);
int set_cgroup_state(const pid_t pid, const int weight);

/***** MAIN PART *****/
int main()
{
	pid_t pids[MAX_TASK];
	int weight[MAX_TASK] = { 0 };
	char bench_file[MAX_TASK][FILENAME_MAX];
	pid_t pid;
	int ret;

	if (getuid() != 0) {
		fprintf(stderr,
			"You have to run this program with superuser privilege\n");
		return -EINVAL;
	}
	ret = system("rmdir /sys/fs/cgroup/blkio/" PREFIX_CGOURP_NAME "*");

	memset(weight, DEFAULT_WEIGHT, sizeof(weight));
	memset(bench_file, 0, sizeof(bench_file));

	ret = read_config_file("config.txt", weight, bench_file);
	if (ret < 0) {
		fprintf(stderr, "File has invalid data....\n");
		return ret;
	}

	TELL_WAIT();
	/* execute the fork sequence... */
	for (int i = 0; i < MAX_TASK; i++) {
		if ((pid = fork()) < 0) { /* fork failed */
			fprintf(stderr, "Fork failed...\n");
		} else if (pid == 0) { /* child process */
			if (bench_exec_process(weight[i], bench_file[i])) {
				return errno;
			}
		}
		/* parent process */
		pids[i] = pid;
	}

	/* parent process */
	for (int i = 0; i < 4; i++) { /* Start trace notify */
		set_cgroup_state(pids[i], weight[i]);
		TELL_CHILD(pids[i]);
	}

	for (int i = 0; i < 4; i++) {
		int status = 0;
		if ((pid = waitpid(pids[i], &status, 0)) < 0) {
			fprintf(stderr, "wait pid failed...\n");
		}
		fprintf(stdout, "process terminated detected %d\n", pid);
	}
	return 0;
}

/***** FUNCTION DEFINITION PART ****/
int read_config_file(const char *file_name, int weight[],
		     char bench_file[MAX_TASK][FILENAME_MAX])
{
	FILE *fp = NULL;
	int ret = 0;
	fp = fopen(file_name, "r");
	if (fp != NULL) {
		for (int i = 0; i < MAX_TASK; i++) {
			if (feof(fp)) {
				ret = -EFAULT;
				break;
			}
			fscanf(fp, "%d %s", &weight[i], bench_file[i]);
			fprintf(stdout, "weight: %d, file: %s\n", weight[i],
				bench_file[i]);
		}
		fclose(fp);
	} else {
		fprintf(stdout, "cannot find config file...\n");
	}
	return ret;
}

int bench_exec_process(int weight, char bench_file[])
{
	char file_name[FILENAME_MAX] = { 0 };
	char *token;
	int fd = 0;

	WAIT_PARENT(); /* wait parent's start signal */
	token = strtok(bench_file, "/");
	token = strtok(NULL, token);
	token = strtok(token, ".");
	sprintf(file_name, "%d_%d_%s.out", getpid(), weight, token);
	fd = open(file_name, O_CREAT | O_WRONLY, 0755);
	dup2(fd, STDOUT_FILENO); /* redirect */
	close(fd);
	sprintf(file_name, "%d_%d_%s.txt", getpid(), weight, token);
	if (execlp("./bin/trace_replay", "trace_replay", "32", "8", file_name,
		   "60", "1", "/dev/nvme0n1", bench_file, "0", "0", "0",
		   (char *)0) < 0) {
		fprintf(stderr, "execlp running failed...(errno: %d)\n", errno);
		return errno;
	}
	return 0;
}

int set_cgroup_state(const pid_t pid, const int weight)
{
	int ret;
	char buffer[sysconf(_SC_ARG_MAX)];

	sprintf(buffer, "mkdir /sys/fs/cgroup/blkio/" PREFIX_CGOURP_NAME "%d",
		pid);
	ret = system(buffer);
	if (ret) {
		fprintf(stderr, "Cannot run command (%s)\n", buffer);
		return -EFAULT;
	}

	sprintf(buffer,
		"echo %d > /sys/fs/cgroup/blkio/" PREFIX_CGOURP_NAME
		"%d/blkio.kyber.weight",
		weight, pid);
	ret = system(buffer);
	if (ret) {
		fprintf(stderr, "Cannot run command (%s)\n", buffer);
		return -EFAULT;
	}

	sprintf(buffer,
		"echo %d > /sys/fs/cgroup/blkio/" PREFIX_CGOURP_NAME "%d/tasks",
		pid, pid);
	ret = system(buffer);
	if (ret) {
		fprintf(stderr, "Cannot run command (%s)\n", buffer);
		return -EFAULT;
	}
	return ret;
}