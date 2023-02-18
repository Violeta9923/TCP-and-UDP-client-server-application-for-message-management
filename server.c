#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <math.h>
#include "utils.h"


void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file); 
	exit(0);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen_UDP, clilen_TCP;

    if (argc < 2) {
		usage(argv[0]);
	}

    int UDPfd = socket(PF_INET, SOCK_DGRAM, 0);
    DIE(UDPfd < 0, "socket_UDP");

    int TCPfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(TCPfd < 0, "socket_TCP");

    int err = atoi(argv[1]);
	DIE(err == 0, "atoi");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(err);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

    err = bind(TCPfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(err < 0, "bind1");

    err = bind(UDPfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(err < 0, "bind2");

    err = listen(TCPfd, MAX_CLIENTS);
	DIE(err < 0, "listen");

    int check = 1;
    err = setsockopt(TCPfd, IPPROTO_TCP, TCP_NODELAY, (char*) &check, sizeof(int));
    DIE(err < 0, "TCPdelay");

    fd_set read_fds;	
    fd_set tmp_fds;
    int fdmax;
    
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

    FD_SET(TCPfd, &read_fds);
    FD_SET(UDPfd, &read_fds);

    if (UDPfd > TCPfd) {
        fdmax = UDPfd;
    } else {
        fdmax = TCPfd;
    }

    client clients[10];

    int number = 0;
    
    while (1){
        tmp_fds = read_fds;

        err = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(err < 0, "select");

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == UDPfd) {
                    // am primit un mesaj de la UDP

                    char buf[BUFFLEN];
                    memset(buf, 0, BUFFLEN);
                    clilen_UDP = sizeof(cli_addr);
                    memset(buf, 0, BUFFLEN);
                    err = recvfrom(UDPfd, buf, BUFFLEN, 0, (struct sockaddr*) 
                        &serv_addr, &clilen_UDP);
                    DIE(err < 0, "recvfrom");

                    // datele despre topica sunt in buf
                    char nume[51];
                    memset(nume, 0, 50);
                    memcpy(nume, buf, 49);
                    nume[50] = '\0';
                    char info[1500];
                    memset(info, 0, 1500);
                    int tip = buf[50] - '0';

                    if (tip == -48) {
                        make_int(info, buf);
                    } else if (tip == -47) {
                        make_short_int(info, buf);
                    } else if (tip == -46) {
                        make_float(info, buf);
                    } else if (tip == -45) {
                        make_string(info, buf);
                    }
                    
                    // construiesc printul conform temei
                    char print[1570];
                    memset(print, 0, 1570);
                    strcat(print, inet_ntoa(serv_addr.sin_addr));
                    strcat(print, ":");
                    char var[10];
                    memset(var, 0, 10);
                    sprintf(var, "%d", serv_addr.sin_port);
                    strcat(print, var);
                    strcat(print, "-");
                    strcat(print, nume);
                    strcat(print, info);
                    
                    for (int j = 0; j < number; j++) {
                        for (int k = 0; k < clients[j].subscribsions; k++) {
                            if (strcmp(nume, clients[j].topics[k]) == 0) {
                                if (clients[j].connected == 1) {
                                    send(clients[j].fd, print, strlen(print), 0);
                                } else if (clients[j].SF == 1) {
                                    strcat(clients[j].unsend, print);
                                    strcat(clients[j].unsend, " / ");
                                }
                                break;
                            }
                        }
                    }
                } else if (i == TCPfd) {
                    // un nou client TCP
					clilen_TCP = sizeof(cli_addr);
					int newfd = accept(TCPfd, (struct sockaddr *) &cli_addr, &clilen_TCP);
					DIE(newfd < 0, "primire TCP nou");

					FD_SET(newfd, &read_fds);
					if (newfd > fdmax) { 
						fdmax = newfd;
                    }
                    char buf[BUFFLEN];
                    memset(buf, 0, BUFFLEN);
                    err = recv(newfd, buf, BUFFLEN, 0);
                    DIE(err < 0, "primire mesaj TCP");

                    int gasit = 0;
                    for (int j = 0; j < number; i++) {
                        if (strcmp(clients[j].id, buf) == 0) {
                            // reconectare
                            gasit = 1;
                            clients[j].connected = 1;
                            send(clients[j].fd, clients[j].unsend, 
                                strlen(clients[j].unsend), 0);
                            memset(clients[j].unsend, 0, strlen(clients[j].unsend));
                            break;
                        }
                    }
                    if (gasit == 0) {
                        // client nou
                        clients[number].fd = newfd;

                        memcpy(clients[number].id, buf, BUFFLEN);

                        clients[number].subscribsions = 0;
                        clients[number].SF = 0;
                        clients[number].connected = 1;
                        number++;

                        printf("New client %s connected from %s:%d.\n", clients[number].id, 
                            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                    }
					
                } else {
                    char buffer[BUFFLEN];
                    memset(buffer, 0, BUFFLEN);
					int n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

                    char command[12];
                    memcpy(command, buffer, 11);

                    if (memcmp(command, "unsubscribe", 11) == 0) {
                        // unsubscribe nume
                        char name[50];
                        memset(name, 0, 50);
                        memcpy(name, buffer + 12, strlen(buffer) - 13);

                        int n = 0;
                        for (int j = 0; j < number; j++) {
                            if (clients[j].fd == i) {
                                for (int m = 0; m < clients[j].subscribsions; i++) {
                                    if (strcmp(clients[j].topics[m], name) == 0) {
                                        n = m;
                                        break;
                                    }
                                }
                                for (int m = n; m < clients[j].subscribsions - 1; i++) {
                                    strcpy(clients[j].topics[m], clients[j].topics[m + 1]);
                                }
                                clients[j].subscribsions--;
                                break;
                            }
                        }

                        printf("unsubscribed %s.\n", name);
                    } else if (memcmp(command, "subscribe", 9) == 0) {
                        // subscribe nume 0/1
                        char name[50];
                        memset(name, 0, 50);
                        memcpy(name, buffer + 10, strlen(buffer) - 13);
                        char SF;
                        SF =  buffer[strlen(buffer) - 2];

                        for (int j = 0; j < number; j++) {
                            if (clients[j].fd == i) {
                                strcpy(clients[j].topics[clients[j].subscribsions], name);
                                if (SF == '1') {
                                    clients[j].SF = 1;
                                } else {
                                    clients[j].SF = 0;
                                }
                                clients[j].subscribsions ++;
                            }
                        }
                        printf("subscribed %s.\n", name);
                    } else if (strlen(command) == 0) {
                        // exit sau ctrl C

                        int k = 0;
                        for (int j = 0; j < number; j++) {
                            if (clients[j].fd == i) {
                                k = j;
                                clients[j].connected = 0;
                                break;
                            }
                        }
                        printf("%s disconnected.\n", clients[k].id);
                        FD_CLR(i, &read_fds);
                        close(i);
                    } else {
                        // nu e bine
                    }
                }
            }
        }
    }

	close(UDPfd);
    close(TCPfd);

	return 0;
}
