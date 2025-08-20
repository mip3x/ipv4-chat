#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../include/common.h"

bool isValidIpAddress(char *ip_addr) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_addr, &(sa.sin_addr));
    return result == 1;
}


void usage(char* progname, int opt) {
    fprintf(stdout, USAGE_FMT, progname);

    switch (opt) {
        case 'a':
            fprintf(stderr, "Incorrect `ipv4_address` argument: value should be correct IPv4 address\n");
            break;

        case 'p':
            fprintf(stderr, "Incorrect `port` argument: value should be integer\n");
            break;

        case 'h':
            exit(EXIT_SUCCESS);

        default:
            fprintf(stderr, "Providing arguments `ipv4_address` and `port` is mandatory\n");
            break;
    }

    exit(EXIT_FAILURE);
}
