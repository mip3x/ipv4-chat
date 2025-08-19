#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define USAGE_FMT "%s [-v] [-a ipv4_address (default 127.0.0.1) ] [-p port (default 42424)] [-h]"

bool isValidIpAddress(char *ip_addr);
void usage(char* progname, int opt);

#endif
