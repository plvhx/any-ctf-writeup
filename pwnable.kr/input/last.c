#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void) {
	int fd;
	int acc;
	struct sockaddr_in sock_handler;

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&sock_handler, 0, sizeof(sock_handler));

	sock_handler.sin_family = AF_INET;
	sock_handler.sin_addr.s_addr = INADDR_ANY;
	sock_handler.sin_port = htons(31337);

	connect(fd, (struct sockaddr *)&sock_handler, (socklen_t)sizeof(sock_handler));

	send(fd, "\xde\xad\xbe\xef", 4, MSG_DONTWAIT);

	close(fd);

	return ( 0 );
}
