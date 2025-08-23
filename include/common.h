#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

#define USAGE_FMT "%s [-v] [-a ipv4_address (default 127.0.0.1) ] [-p port (default 42424)] [-h]\n"
#define IP_ADDR_LEN 16
#define MANDATORY_ARGS_NUMBER 2
#define OPTIONAL_ARGS_NUMBER 2

typedef struct {
    int verbose;
    char *ip_addr;
    uint16_t port;
} options_t;

typedef struct {
    char flag;
    char *description;
} argument_t;

bool is_valid_ip_addr(char *ip_addr);
void print_error(const char *fmt, ...);
void usage(char* progname, int opt);

#endif
