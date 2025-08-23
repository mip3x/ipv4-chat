#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/ipv4_chat.h"

#define OPTSTR "va:p:h"

extern int errno;
extern char* optarg;
extern int opterr, optind;

int main(int argc, char* argv[]) {
    int opt;
    options_t options = { 0, NULL, 0 };

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
        switch (opt) {
            case 'v':
                options.verbose += 1;
                break;

            case 'a':
                if (is_valid_ip_addr(optarg)) {
                    options.ip_addr = malloc(sizeof(char) * IP_ADDR_LEN);
                    strcpy(options.ip_addr, optarg);
                    break;
                }
                usage(argv[0], opt);
                break;

            case 'p':
                options.port = atoi(optarg);
                if (errno != 0)
                    usage(argv[0], opt);

                break;

            case 'h':
            default:
                usage(argv[0], opt);
                break;
        }
    }

    if (options.ip_addr == NULL || options.port == 0) {
        usage(argv[0], opt);
    }

    printf("verbose flag: %b\n", options.verbose);
    printf("ip_addr: %s\n", options.ip_addr);
    printf("port: %d\n", options.port);

    struct ipv4_chat ipv4_chat = { .nickname = NULL, .options = options };
    if (ipv4_chat_handshake(&ipv4_chat) == -1)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
