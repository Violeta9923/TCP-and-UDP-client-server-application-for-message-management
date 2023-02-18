#ifndef ENGINE_H_
#define ENGINE_H_
#include <stdio.h>
#include <stdlib.h>

#define BUFFLEN 1000
#define MAXTITLE 50
#define MAX_CLIENTS	200

// printeaza erorile si inchide procesul
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)



typedef  struct {
	uint32_t a;
	uint32_t b;
}pair;

// structura client retine toate datele cu care lucrez in server
typedef struct {
    int fd;
    char id[10];
	char topics[MAXTITLE][MAXTITLE];
	int subscribsions;
	int SF;
	char unsend[1600];
	int connected;
}client;

void make_int(char *info, char* buf);
void make_short_int(char *info, char* buf);
void make_float(char *info, char* buf);
void make_string(char *info, char* buf);

#endif