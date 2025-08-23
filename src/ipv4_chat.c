#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/ipv4_chat.h"

#define HANDSHAKE_MS 700

static int create_socket_bind(const char *bind_ip, const uint16_t bind_port,
                                  struct sockaddr_in *bind_addr, 
                                  struct sockaddr_in *broadcast_addr) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) return -1;

    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(fd);
        return -1;
    }

    memset(bind_addr, 0, sizeof *bind_addr);
    bind_addr->sin_family = AF_INET;
    bind_addr->sin_port = htons(bind_port);

    if (inet_pton(AF_INET, bind_ip, &bind_addr->sin_addr) != 1) {
        print_error("inet_pton: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr*)bind_addr, sizeof *bind_addr) == -1) {
        print_error("bind: %s", strerror(errno));
        close(fd);
        return -1;
    }

    memset(bind_addr, 0, sizeof *broadcast_addr);
    broadcast_addr->sin_family = AF_INET;
    broadcast_addr->sin_port = htons(0);
    broadcast_addr->sin_addr.s_addr = htonl(INADDR_BROADCAST);

    return fd;
}

static void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int enter_nickname(char *nickname) {
    for (;;) {
        printf("Enter nickname (1-%d symbols, no space symbols): ", NICKNAME_MAX_LEN);
        fflush(stdout);

        char nickname_buf[NICKNAME_MAX_LEN + 1];
        if (!fgets(nickname_buf, sizeof nickname_buf, stdin)) return -1;

        size_t len = strcspn(nickname_buf, "\n");
        int eol = (nickname_buf[len] == '\n');
        nickname_buf[len] = '\0'; 

        if (!eol) {
            clear_stdin();
            print_error("The nickname must be no longer than %d characters!", NICKNAME_MAX_LEN);
            continue;
        }

        if (len == 0) {
            print_error("The nickname must not be empty!");
            continue;
        }

        bool contains_space_sym = false;
        for (size_t i = 0; i < len; ++i) {
            if (isspace((unsigned char)nickname_buf[i])) {
                contains_space_sym = true;
                break;
            }
        }

        if (contains_space_sym) {
            print_error("The nickname must not contain space symbols!");
            continue;
        }

        strcpy(nickname, nickname_buf);

        return 0;
    }
}

int ipv4_chat_handshake(struct ipv4_chat *chat) {
    if (!chat) return -1;

    struct sockaddr_in bind_addr, broadcast_addr;
    int fd = create_socket_bind(chat->options.ip_addr, chat->options.port, &bind_addr, &broadcast_addr);
    if (fd == -1) return -1;

    char nickname[NICKNAME_MAX_LEN];
    if (enter_nickname(nickname) == -1) {
        print_error("Failure entering nickname!");
        return -1;
    }

    chat->nickname = strdup(nickname);
    printf("Entered nickname: %s\n", chat->nickname);

    return 0;
}
