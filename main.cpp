#include <iostream>
#include <list>

/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "TCP_Header.h"

#define DEFAULT_PORT        9090
#define MAX_WINDOW_SIZE     10
#define ACK_RESPONSE_TIME   5

using namespace std;

void error(const char *);
void send_ACK_msg(list<unsigned>&);

unsigned window = MAX_WINDOW_SIZE;
int sock, n;
size_t length;
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from;

int main(int argc, char *argv[]) {
    char buf[HEADER_SIZE];
    list<unsigned> window_list;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        error("Opening socket");

    length = sizeof(server);
    bzero(&server, length);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);
    if (bind(sock, (struct sockaddr *)&server, static_cast<socklen_t>(length)) < 0)
        error("binding");
    fromlen = sizeof(struct sockaddr_in);

    while (true) {
        n = recvfrom(sock, buf, HEADER_SIZE, 0, (struct sockaddr *)& from, &fromlen);
        if (n < 0)
            error("recvfrom");
        TCP_Header tcp_header(buf);
        window_list.push_back(tcp_header.getSequence());
    }
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/**
 * This is meant to be runned by a secundary thread.
 */
void send_ACK_msg(list<unsigned> &window_list) {
    n = sendto(sock, "Got your message\n", HEADER_SIZE, 0, (struct sockaddr *)& from, fromlen);
    if (n  < 0)
        error("sendto");
    // TODO terminar esto
}