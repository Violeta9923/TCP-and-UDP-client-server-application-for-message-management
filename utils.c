#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "utils.h"

// preia din buf mesajul si il introduce in info ca signed int
void make_int(char *info, char* buf) {
    strcpy(info, "-INT-[");
    uint8_t semn = buf[51];
    if (semn == 1) {
        strcat(info, "-");
    }
    uint32_t numar;
    memcpy(&numar, buf + 52, sizeof(uint32_t));
    numar = ntohl(numar);
    char var[10];
    memset(var, 0, 10);
    sprintf(var, "%d", numar);
    strcat(info, var);
    strcat(info, "]");
}

// preia din buf mesajul si il introduce in info ca signed short
void make_short_int(char *info, char* buf) {
    strcat(info, " - SHORT_REAL-[");
    uint16_t numar;
    memcpy(&numar, buf + 51, sizeof(uint16_t));
    numar = ntohs(numar);
    float alt_numar = ((float)numar)/100;
    char var[10];
    memset(var, 0, 10);
    sprintf(var, "%f", alt_numar);
    strcat(info, var);
    strcat(info, "]");
}

// preia din buf mesajul si il introduce in info ca float
void make_float(char *info, char* buf) {
    strcat(info, "-FLOAT-[");
    uint8_t semn = buf[51];
    if (semn == 1) {
        strcat(info, "-");
    }
    uint32_t numar;
    memcpy(&numar, info + 52, sizeof(uint32_t));
    numar = ntohl(numar);
    uint8_t exp = info[56];
    float alt_numar = ((float) numar) * pow(10, -exp);
    char var[10];
    memset(var, 0, 10);
    sprintf(var, "%f", alt_numar);
    strcat(info, var);
    strcat(info, "]");
}

// preia din buf mesajul si il introduce in info ca string
void make_string(char *info, char* buf) {
    strcat(info, "-STRING-[");
    strcat(info, buf + 51);
    strcat(info, "]");
}
