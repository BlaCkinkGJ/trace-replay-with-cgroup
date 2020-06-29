/**
 *  sync.h: synchronize functions in parent and children
 *  Copyright (C) 2020 BlaCkinkGJ
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SYNC_H
#define SYNC_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* 0: read 1: write*/
static int pfd1[2], pfd2[2];

int TELL_WAIT(void)
{
	if (pipe(pfd1) < 0 || pipe(pfd2) < 0) {
		fprintf(stderr, "[%s] pipe error...\n", __func__);
		return -EPIPE;
	}
	return 0;
}

int TELL_PARENT(pid_t pid)
{
	if (write(pfd2[1], "c", 1) != 1) {
		fprintf(stderr, "[%s] write error...\n", __func__);
		return -EIO;
	}
	return 0;
}

int WAIT_PARENT(void)
{
	char recv;

	if (read(pfd1[0], &recv, 1) != 1) {
		fprintf(stderr, "[%s] read error...\n", __func__);
		return -EIO;
	}

	if (recv != 'p') {
		fprintf(stderr, "[%s] incorrect data received(%c)\n", __func__,
			recv);
		return -EIO;
	}
	return 0;
}

int TELL_CHILD(pid_t pid)
{
	if (write(pfd1[1], "p", 1) != 1) {
		fprintf(stderr, "[%s] write error...\n", __func__);
		return -EIO;
	}
	return 0;
}

int WAIT_CHILD(void)
{
	char recv;

	if (read(pfd2[0], &recv, 1) != 1) {
		fprintf(stderr, "[%s] read error...\n", __func__);
		return -EIO;
	}

	if (recv != 'c') {
		fprintf(stderr, "[%s] incorrect data received(%c)\n", __func__,
			recv);
		return -EIO;
	}
	return 0;
}

#endif
