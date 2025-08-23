#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>
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

static argument_t mandatory_arguments[MANDATORY_ARGS_NUMBER] = {
    { 'a', "[address]: ipv4 address where the program will expect to receive messages from its other copies" },
    { 'p', "[port]: similar to [address], but port" },
};

static argument_t optional_arguments[OPTIONAL_ARGS_NUMBER] = {
    { 'v', "[verbose]: detailed output" },
    { 'h', "[help]: output auxiliary information" },
};

static void printMandatoryArguments() {
    fprintf(stdout, "Mandatory arguments:\n");

    for (size_t i = 0; i < MANDATORY_ARGS_NUMBER; i++) {
        fprintf(stdout, 
                "\t-%c\t%s\n",
                mandatory_arguments[i].flag,
                mandatory_arguments[i].description);
    }
}

static void printOptionalArguments() {
    fprintf(stdout, "Optional arguments:\n");

    for (size_t i = 0; i < OPTIONAL_ARGS_NUMBER; i++) {
        fprintf(stdout, 
                "\t-%c\t%s\n",
                optional_arguments[i].flag,
                optional_arguments[i].description);
    }
}

static void printHelpMessage() {
    fprintf(stdout, "Start IPv4 chat session. Chat works only in Local Area Network (LAN) via UDP broadcast.\n\n");
    printMandatoryArguments();
    fprintf(stdout, "\n");
    printOptionalArguments();
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
            printHelpMessage();
            exit(EXIT_SUCCESS);

        default:
            fprintf(stderr, "Providing arguments `ipv4_address` and `port` is mandatory\n");
            break;
    }

    exit(EXIT_FAILURE);
}
