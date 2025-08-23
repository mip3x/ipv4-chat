#ifndef IPV4_CHAT
#define IPV4_CHAT

#include "common.h"

#define NICKNAME_MAX_LEN 50
#define CHAT_MSG_MAX_LEN 1000

struct ipv4_chat {
    char *nickname;
    options_t options;
};

int ipv4_chat_handshake(struct ipv4_chat *chat);

#endif
