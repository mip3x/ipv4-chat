#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <uchar.h>

#define USAGE_FMT "%s [-v] [-a ipv4_address (default 127.0.0.1) ] [-p port (default 42424)] [-h]\n"
#define IP_ADDR_LEN 16
#define MANDATORY_ARGS_NUMBER 2
#define OPTIONAL_ARGS_NUMBER 2

typedef struct {
    char flag;
    char* description;
} argument_t;

bool isValidIpAddress(char *ip_addr);
void usage(char* progname, int opt);

#endif
