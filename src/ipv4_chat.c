#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/ipv4_chat.h"
#include "../include/listener.h"
#include "../include/sender.h"

#define HANDSHAKE_MS 700

static int get_broadcast_by_ip(const char *ip_str, struct in_addr *out_bcast) {
    if (!ip_str || !out_bcast) {
        perror("get_broadcast_by_ip: arg is NULL");
        return -1;
    }

    struct in_addr target;
    if (inet_pton(AF_INET, ip_str, &target) != 1) {
        print_error("inet_pton: %s", strerror(errno));
        return -1;
    }

    struct ifaddrs *interfaces = NULL;
    if (getifaddrs(&interfaces) == -1) {
        print_error("getifaddrs: %s", strerror(errno));
        return -1;
    }

    for (struct ifaddrs *iter = interfaces; iter; iter = iter->ifa_next) {
        if (!iter->ifa_addr || iter->ifa_addr->sa_family != AF_INET)
            continue;

        struct sockaddr_in *interface_addr = (struct sockaddr_in*)iter->ifa_addr;
        if (interface_addr->sin_addr.s_addr != target.s_addr)
            continue;

        if (iter->ifa_netmask && iter->ifa_netmask->sa_family == AF_INET) {
            struct sockaddr_in *interface_netmask = (struct sockaddr_in*)iter->ifa_netmask;
            uint32_t addr = ntohl(interface_addr->sin_addr.s_addr);
            uint32_t mask = ntohl(interface_netmask->sin_addr.s_addr);

            uint32_t bcast = (addr & mask) | (~mask);
            out_bcast->s_addr = htonl(bcast);
            break;
        }
    }

    freeifaddrs(interfaces);
    return 0;
}

static int setup_sockets(const char *host_ip,
                         const uint16_t port,
                         struct sockaddr_in *bind_addr, 
                         struct sockaddr_in *broadcast_addr,
                         bool verbose) {
    int bind_fd;
    if ((bind_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        print_error("socket: %s", strerror(errno));
        return -1;
    }

    int yes = 1;
    if (setsockopt(bind_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(bind_fd);
        return -1;
    }

    if (setsockopt(bind_fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(bind_fd);
        return -1;
    }

    if (setsockopt(bind_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == -1) {
        print_error("setsockopt: %s", strerror(errno));
        close(bind_fd);
        return -1;
    }

    memset(bind_addr, 0, sizeof *bind_addr);
    bind_addr->sin_family = AF_INET;
    bind_addr->sin_port = htons(port);
    bind_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(bind_fd, (struct sockaddr*)bind_addr, sizeof *bind_addr) == -1) {
        print_error("bind: %s", strerror(errno));
        close(bind_fd);
        return -1;
    }

    memset(broadcast_addr, 0, sizeof *broadcast_addr);
    broadcast_addr->sin_family = AF_INET;
    broadcast_addr->sin_port = htons(port);

    struct in_addr network_broadcast;
    if (get_broadcast_by_ip(host_ip, &network_broadcast) == -1) {
        close(bind_fd);
        return -1;
    }
    broadcast_addr->sin_addr = network_broadcast;

    if (verbose) {
        char bcbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &network_broadcast, bcbuf, sizeof bcbuf);
        printf("broadcast resolved: %s:%u\n", bcbuf, port);
    }

    return bind_fd;
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

static void status_line(const char *msg, int step) {
    static size_t prev_len = 0;
    static const char* dots[] = { "", ".", "..", "..." };

    char line[256];
    snprintf(line, sizeof line, "%s%s", msg, dots[step % 4]);
    size_t len = strlen(line);

    printf("\r%s", line);
    if (prev_len > len) {
        printf("%*s", (int)(prev_len - len), "");
        printf("\r%s", line);
    }
    fflush(stdout);

    prev_len = len;
}

static void status_clear(void) {
    static size_t prev_len = 0;
    (void)prev_len;
    printf("\r%*s\r", 40, "");
    fflush(stdout);
}

static bool wait_for_handshakes(int fd, const char *host_ip) {
    struct pollfd handshake_pfd = { .fd = fd, .events = POLLIN };
    char buf[MSG_MAX_LEN];
    bool conflict = false;

    int remaining = HANDSHAKE_MS;
    int poll_timeout = 100;
    uint8_t dots_count = 1;

    while (remaining > 0) {
        status_line("Checking uniqueness of the nickname", dots_count++);

        poll_timeout = poll_timeout < remaining ? poll_timeout : remaining;
        int ready_fds = poll(&handshake_pfd, 1, poll_timeout);
        remaining -= poll_timeout;

        if (ready_fds <= 0) continue;

        struct sockaddr_in hs_src;
        socklen_t hs_src_len = sizeof hs_src;
        ssize_t rcv_msg_len = recvfrom(fd,
                                       buf,
                                       sizeof(buf) - 1,
                                       0,
                                       (struct sockaddr*)&hs_src,
                                       &hs_src_len);

        if (rcv_msg_len <= 0) continue;
        buf[rcv_msg_len] = '\0';

        struct in_addr host;
        if (inet_pton(AF_INET, host_ip, &host) != 1) return false;
        if (hs_src.sin_addr.s_addr == host.s_addr) continue;

        if (strncmp(buf, "NOT_OK", 6) == 0) {
            conflict = true;
            break;
        }
    }

    status_clear();

    return conflict;
}

int run(struct ipv4_chat *chat) {
    if (!chat->nickname) {
        print_error("Nickname is not set!");
        return -1;
    }

    volatile int stop = 0;
    pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;
    chat->stop = &stop;
    chat->print_mtx = &print_mtx;

    pthread_t th_listener, th_sender;

    if (pthread_create(&th_listener, NULL, listener_thread, chat) != 0) {
        print_error("pthread_create(listener): %s", strerror(errno));
        close(chat->bind_fd);
        return -1;
    }

    if (pthread_create(&th_sender, NULL, sender_thread, chat) != 0) {
        print_error("pthread_create(sender): %s", strerror(errno));
        stop = 1;
        pthread_join(th_listener, NULL);
        close(chat->bind_fd);
        return -1;
    }

    pthread_join(th_sender, NULL);
    stop = 1;
    pthread_join(th_listener, NULL);
    close(chat->bind_fd);

    return 0;
}

int nickname_handshake(struct ipv4_chat *chat) {
    if (!chat) return -1;

    chat->bind_fd = setup_sockets(chat->options.ip_addr,
                                  chat->options.port,
                                  &chat->bind_addr,
                                  &chat->broadcast_addr,
                                  chat->options.verbose);
    if (chat->bind_fd == -1) return -1;

    for (;;) {
        char nickname_candidate[NICKNAME_MAX_LEN];
        if (enter_nickname(nickname_candidate) == -1) {
            close(chat->bind_fd);
            print_error("Failure entering nickname!");
            return -1;
        }

        char probe[sizeof nickname_candidate + sizeof NICKNAME_HANDSHAKE_Q];
        snprintf(probe, sizeof probe, "%s %s", NICKNAME_HANDSHAKE_Q, nickname_candidate);
        if (sendto(chat->bind_fd,
                   probe,
                   strlen(probe),
                   0,
                   (const struct sockaddr*)&chat->broadcast_addr,
                   sizeof(chat->broadcast_addr)) == -1) {
            print_error("sendto[nickname_handshake]: %s", strerror(errno));
            close(chat->bind_fd);
            return -1;
        }

        bool conflict = wait_for_handshakes(chat->bind_fd, chat->options.ip_addr);
        if (conflict) {
            print_error("Nickname '%s' is already in use. Choose another one!", nickname_candidate);
            continue;
        }
        if (chat->options.verbose) printf("Nickname '%s' is free\n", nickname_candidate);

        chat->nickname = strdup(nickname_candidate);
        if (!chat->nickname) {
            print_error("strdup: %s", strerror(errno));
            close(chat->bind_fd);
            return -1;
        }
        if (chat->options.verbose) printf("Your nickname is '%s' now\n", chat->nickname);

        return 0;
    }

    return 0;
}
