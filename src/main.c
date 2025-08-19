#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/common.h"

#define OPTSTR "va:p:h"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 42424

extern int errno;
extern char* optarg;
extern int opterr, optind;

typedef struct {
    int verbose;
    char *ip_addr;
    uint16_t port;
} options_t;

int main(int argc, char* argv[]) {
    int opt;
    options_t options = { 0, DEFAULT_IP, DEFAULT_PORT };

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
        switch (opt) {
            case 'v':
                options.verbose += 1;
                break;

            case 'a':
                if (isValidIpAddress(optarg)) {
                    strcpy(options.ip_addr, optarg);
                    printf("ipv4_addr: \n%s", optarg);
                    break;
                }
                usage(argv[0], opt);
                break;

            case 'p':
                options.port = atoi(optarg);
                if (errno != 0)
                    usage(argv[0], opt);

                printf("port: %d\n", options.port);
                break;

            case 'h':
            default:
                usage(argv[0], opt);
                break;
        }
    }

    return EXIT_SUCCESS;
}
