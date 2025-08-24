#include <arpa/inet.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../include/common.h"

void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

bool is_valid_ip_addr(char *ip_addr) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_addr, &(sa.sin_addr));
    return result == 1;
}

void print_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Error! ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}

static argument_t mandatory_arguments[MANDATORY_ARGS_NUMBER] = {
    { 'a', "[address]: ipv4 address where the program will expect to receive messages from its other copies" },
    { 'p', "[port]: similar to [address], but port" },
};

static argument_t optional_arguments[OPTIONAL_ARGS_NUMBER] = {
    { 'v', "[verbose]: detailed output" },
    { 'h', "[help]: output auxiliary information" },
};

static void print_mandatory_args() {
    fprintf(stdout, "Mandatory arguments:\n");

    for (size_t i = 0; i < MANDATORY_ARGS_NUMBER; i++) {
        fprintf(stdout, 
                "\t-%c\t%s\n",
                mandatory_arguments[i].flag,
                mandatory_arguments[i].description);
    }
}

static void print_optional_args() {
    fprintf(stdout, "Optional arguments:\n");

    for (size_t i = 0; i < OPTIONAL_ARGS_NUMBER; i++) {
        fprintf(stdout, 
                "\t-%c\t%s\n",
                optional_arguments[i].flag,
                optional_arguments[i].description);
    }
}

static void print_help_msg() {
    fprintf(stdout, "Start IPv4 chat session. Chat works only in Local Area Network (LAN) via UDP broadcast.\n\n");
    print_mandatory_args();
    fprintf(stdout, "\n");
    print_optional_args();
}

void usage(char* progname, int opt) {
    fprintf(stdout, USAGE_FMT, progname);

    switch (opt) {
        case 'a':
            print_error("Incorrect `ipv4_address` argument: value should be correct IPv4 address");
            break;

        case 'p':
            print_error("Incorrect `port` argument: value should be integer from 1 to 65535");
            break;

        case 'h':
            print_help_msg();
            exit(EXIT_SUCCESS);

        default:
            print_error("Providing arguments `ipv4_address` and `port` is mandatory");
            break;
    }

    exit(EXIT_FAILURE);
}
