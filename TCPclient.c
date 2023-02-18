#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.h"

int main(int argc, char *argv[])
{
	int sockfd, err;
	struct sockaddr_in serv_addr;
	char buffer[BUFFLEN];

	if (argc < 4) {
		fprintf(stderr, "Usage: ./subscriber <ID_Client> <IP_Server> <Port_Server>\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	// initializeaza serverul
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	err = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(err == 0, "inet_aton");

	err = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(err < 0, "connect");

	// defineste multimile socket-ilor
	fd_set read_fds;
	fd_set tmp_fds;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	// trimite id-ul clientului
	memset(buffer, 0, 10);
	memcpy(buffer, argv[1], strlen(argv[1]));

	err = send(sockfd, buffer, strlen(buffer), 0);
	DIE(err < 0, "send");

	while (1) {
		tmp_fds = read_fds;

		err = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(err < 0, "select");

		if (FD_ISSET(sockfd, &tmp_fds)) {
			// primeste mesaj de la server
			memset(buffer, 0, BUFFLEN);
			recv(sockfd, buffer, BUFFLEN, 0);
			printf("%s\n", buffer);

		} else if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			// se scrie comanda in terminal 
			memset(buffer, 0, BUFFLEN);
			fgets(buffer, BUFFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			// trimitem comanda
			err = send(sockfd, buffer, strlen(buffer), 0);
			DIE(err < 0, "send");
		}
	}

	close(sockfd);

	return 0;
}
