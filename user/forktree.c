// Fork a binary tree of processes and display their structure.

#include <inc/lib.h>

#define DEPTH 10

void forktree(const char *cur);

void
forkchild(const char *cur, char branch)
{
	char nxt[DEPTH + 1];

	if (strlen(cur) >= DEPTH)
		return;

	snprintf(nxt, DEPTH + 1, "%s%c", cur, branch);
	if (fork() == 0) {
		forktree(nxt);
		exit();
	}
}

void
forktree(const char *cur)
{
	//sys_set_priority(strlen(cur) % 5);
	cprintf("%08x: I am '%s'\n", sys_getenvid(), cur);

	forkchild(cur, '0');
	forkchild(cur, '1');
}

void
umain(int argc, char **argv)
{
	forktree("");
}

