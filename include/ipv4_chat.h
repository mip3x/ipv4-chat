#ifndef IPV4_CHAT
#define IPV4_CHAT

#include <pthread.h>
#include <netinet/in.h>

#include "common.h"

#define NICKNAME_MAX_LEN 50
#define MSG_MAX_LEN 1000
#define NICKNAME_HANDSHAKE_Q "IS-NICK-FREE?"
#define MSG_START_IDENTIFIER "MSG"

struct ipv4_chat {
    struct sockaddr_in bind_addr, broadcast_addr;
    int bind_fd;
    char *nickname;
    volatile int *stop;
    pthread_mutex_t *print_mtx;
    options_t options;
};

int nickname_handshake(struct ipv4_chat *chat);
int run(struct ipv4_chat *chat);

#endif
