#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <algorithm>

/* UDP client in the internet domain */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "TCP_Header.h"

#define DEFAULT_PORT        9090
#define MAX_WINDOW_SIZE     10
#define ACK_RESPONSE_TIME   5
#define PACKET_SIZE         1

using namespace std;

void error(const char *);
void send_ACK_msg(list<unsigned>&);
bool contains(const list<unsigned>&, unsigned);

unsigned short window = MAX_WINDOW_SIZE;
unsigned initial_sequence = 1;
int sock;
size_t length;
ssize_t n;
socklen_t fromlen;
mutex my_mutex;
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

    thread replier(send_ACK_msg, ref(window_list));
    while (true) {
        n = recvfrom(sock, buf, HEADER_SIZE, 0, (struct sockaddr *)& from, &fromlen);
        if (n < 0)
            error("recvfrom");
        TCP_Header tcp_header(buf);
        unsigned sequence_number = tcp_header.getSequence();

        my_mutex.lock();
        // If the message has already been received then it is discarded.
        if (!contains(window_list, sequence_number)) {
            window_list.push_back(sequence_number);
        }
        my_mutex.unlock();
    }
}

/**
 * Sends a customized error message.
 * @param msg
 */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/**
 * This is meant to be runned by a secundary thread. Its in charge of sending ACKs
 * each 5 seconds.
 */
void send_ACK_msg(list<unsigned> &window_list) {
    srand (static_cast<unsigned int>(time(nullptr)));
    int randNumber;
    while (true) {
        this_thread::sleep_for(chrono::seconds(ACK_RESPONSE_TIME)); // Wait 5 seconds
        my_mutex.lock();
        /**
         * Get the next ACK_NUMBER to request.
         */
        window_list.sort();
        unsigned ack_Number = initial_sequence;
        auto it = window_list.begin();
        if (!window_list.empty()) {
            while (it != window_list.end()){
                if (ack_Number == (*it)){
                    ack_Number += PACKET_SIZE;
                    ++it;
                } else{
                    break;
                }
            }
        }
        initial_sequence = ack_Number;

        /**
         * Reduce the size of the window if received very few messages or
         * increase it otherwise.
         */
        if (window_list.size() < (window/2)){
            window /= 2;
        } else if (window < MAX_WINDOW_SIZE){
            window++;
        }

        /**
         * Remove from the window any sequence number lower than the next
         * ACK_NUMBER to request.
         */
        bool erased_all = false;
        it = window_list.begin();
        while (!erased_all){
            if (!window_list.empty() && ((*it) < ack_Number)){
                window_list.erase(it++);
            } else{
                erased_all = true;
            }
        }
        my_mutex.unlock();

        randNumber = rand()%10;
        if (randNumber < 8) {
            TCP_Header ack_header(0, ack_Number, window, true);
            char *message = ack_header.header_to_Array();
            n = sendto(sock, message, HEADER_SIZE, 0, (struct sockaddr *) &from, fromlen);
            if (n < 0)
                error("sendto");
            delete[] message;
        }
    }
}

/**
 * Method that checks whether or not a list contains a specific element.
 * @param list: the list to check.
 * @param x: the element to search for.
 * @return true if the element was found, false otherwise.
 */
bool contains(const list<unsigned > &list, unsigned x)
{
    return std::find(list.begin(), list.end(), x) != list.end();
}