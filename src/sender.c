#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ipv4_chat.h"

static ssize_t read_user_line(char *data, size_t cap) {
    if (!fgets(data, cap, stdin))
        return -1;

    size_t len = strcspn(data, "\n");
    int eol = data[len] == '\n';
    data[len] = '\0';

    if (!eol) clear_stdin();
    
    return (ssize_t)len;
}

void *sender_thread(void *argument) {
    struct ipv4_chat *chat = argument;

    char inputbuf[MSG_MAX_LEN];

    pthread_mutex_lock(chat->print_mtx);
    printf("To exit from chat print '/exit':\n");
    pthread_mutex_unlock(chat->print_mtx);

    while (!*chat->stop) {
        pthread_mutex_lock(chat->print_mtx);
        printf(">> ");
        fflush(stdout);
        pthread_mutex_unlock(chat->print_mtx);

        ssize_t len = read_user_line(inputbuf, sizeof inputbuf);
        if (len < 0) {
            *chat->stop = 1;
            break;
        }

        if (strcmp(inputbuf, "/exit") == 0) {
            *chat->stop = 1;
            break;
        }

        if (len == 0) continue;

        size_t packet_size = sizeof(MSG_START_IDENTIFIER) + 1 +
            strlen(chat->nickname) + 1 +
            (size_t)len;

        char *packet = malloc(packet_size);
        if (!packet) continue;

        snprintf(packet, packet_size, "%s %s %s",
                 MSG_START_IDENTIFIER, chat->nickname, inputbuf);

        ssize_t n =sendto(chat->bind_fd, packet, strlen(packet), 0,
               (const struct sockaddr*)&chat->broadcast_addr, sizeof(chat->broadcast_addr));
        if (n == -1) {
            perror("sendto br failed");
        } else {
            puts("sending msg to bc...");
        }

        free(packet);
    }

    return NULL;
}
