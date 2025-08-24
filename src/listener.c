#include <arpa/inet.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

#include "../include/ipv4_chat.h"

#define LISTEN_TIMEOUT 100

static void safe_print(pthread_mutex_t *mtx, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    pthread_mutex_lock(mtx);
    vprintf(fmt, ap);
    fflush(stdout);
    pthread_mutex_unlock(mtx);

    va_end(ap);
}

static void send_text(int fd, const struct sockaddr_in *dst, const char *text) {
    if (sendto(fd,
               text,
               strlen(text),
               0,
               (const struct sockaddr*)dst,
               sizeof(*dst)) == -1) {
        print_error("sendto[send_text]: %s", strerror(errno));
    };
}

static void handle_probe_is_free(struct ipv4_chat *chat,
                                 const struct sockaddr_in *sender,
                                 const char *buf) {
    const char *bufp = buf;
    size_t qlen = strlen(NICKNAME_HANDSHAKE_Q);
    if (strncmp(bufp, NICKNAME_HANDSHAKE_Q, qlen) != 0)
        return;
    bufp += qlen;

    while (*bufp == ' ')
        ++bufp;

    char sender_nickname[NICKNAME_MAX_LEN + 1] = {0};
    size_t iter = 0;
    while (*bufp && *bufp != '\r' && *bufp != '\n' && iter < NICKNAME_MAX_LEN)
        sender_nickname[iter++] = *bufp++;
    sender_nickname[iter] = '\0';

    if (!sender_nickname[0])
        return;

    const char *reply = (strcmp(chat->nickname, sender_nickname) == 0) ? "NOT_OK" : "OK";

    if (chat->options.verbose) printf("Sending %s to %s\n", reply, inet_ntoa(sender->sin_addr));
    send_text(chat->bind_fd, sender, reply);
}

static int parse_msg(const char *buf, char nickname[NICKNAME_MAX_LEN + 1], char text[MSG_MAX_LEN + 1]) {
    const char *pbuf = buf;
    size_t idlen = strlen(MSG_START_IDENTIFIER);
    if (strncmp(pbuf, MSG_START_IDENTIFIER, idlen) != 0)
        return -1;
    pbuf += idlen;

    while (*pbuf == ' ')
        ++pbuf;

    size_t iter = 0;
    while (*pbuf && !isspace((unsigned char)*pbuf) && iter < NICKNAME_MAX_LEN)
        nickname[iter++] = *pbuf++;
    nickname[iter] = '\0';

    if (!nickname[0])
        return -1;

    while (*pbuf == ' ')
        ++pbuf;

    iter = 0;
    while (*pbuf && *pbuf != '\r' && *pbuf != '\n' && iter < MSG_MAX_LEN)
        text[iter++] = *pbuf++;
    text[iter] = '\0';

    return 0;
}

void *listener_thread(void *argument) {
    struct ipv4_chat *chat = argument;

    struct in_addr host_ip;
    if (inet_pton(AF_INET, chat->options.ip_addr, &host_ip) != 1) {
        print_error("inet_pton[listener_thread]: %s", strerror(errno));
        return NULL;
    }

    char rcv_msg_buf[MSG_MAX_LEN + NICKNAME_MAX_LEN];
    char nickname[NICKNAME_MAX_LEN + 1];
    char text[MSG_MAX_LEN + 1];

    struct pollfd pfd = { .fd = chat->bind_fd, .events = POLLIN };

    while (!*chat->stop) {
        int ready_fds = poll(&pfd, 1, LISTEN_TIMEOUT);
        if (ready_fds <= 0) continue;

        struct sockaddr_in sender;
        socklen_t sender_len = sizeof sender;
        ssize_t rcv_msg_len = recvfrom(chat->bind_fd,
                                       rcv_msg_buf,
                                       sizeof(rcv_msg_buf) - 1,
                                       0,
                                       (struct sockaddr*)&sender,
                                       &sender_len);

        if (rcv_msg_len <= 0) continue;
        rcv_msg_buf[rcv_msg_len] = '\0';

        if (strncmp(rcv_msg_buf, NICKNAME_HANDSHAKE_Q, strlen(NICKNAME_HANDSHAKE_Q)) == 0) {
            handle_probe_is_free(chat, &sender, rcv_msg_buf);
            continue;
        }

        if (parse_msg(rcv_msg_buf, nickname, text) == 0) {
            char sender_ip[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &sender.sin_addr, sender_ip, sizeof sender_ip) == NULL)
                continue;

            if (strncmp(chat->nickname, nickname, strlen(nickname)) != 0)
                safe_print(chat->print_mtx, "\n[%s] <%s>: %s\n>> ", sender_ip, nickname, text);
        } 
    }

    return NULL;
}
