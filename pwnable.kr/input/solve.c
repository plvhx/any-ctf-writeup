#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
	char *argv[101] = { "./input", [1 ... 99] = "\x41", NULL };
	char *env[2] = { "\xde\xad\xbe\xef=\xca\xfe\xba\xbe", NULL };
	int fd0[2];
	int fd1[2];

	FILE *foo;

	foo = fopen("\x0a", "wb+");

	fwrite("\x00\x00\x00\x00", 4, 1, foo);

	fclose(foo);

	argv[0x41] = "\x00";
	argv[0x42] = "\x20\x0a\x0d";
	argv[0x43] = "31337";

	pipe(fd0);
	pipe(fd1);

	pid_t child = fork();

	if ( child == 0 ) {
		close(fd0[1]);
		close(fd1[1]);

		dup2(fd0[0], 0);
		dup2(fd1[0], 2);

		close(fd0[0]);
		close(fd1[0]);

		execve("./input", argv, env);
	}
	else {
		close(fd0[0]);
		close(fd1[0]);

		write(fd0[1], "\x00\x0a\x00\xff", 4);
		write(fd1[1], "\x00\x0a\x02\xff", 4);
	}

	return ( 0 );
}
