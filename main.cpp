#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <algorithm>
#include <condition_variable>

/* UDP client in the internet domain */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "TCP_Header.h"

#define DEFAULT_PORT        9090
#define MAX_WINDOW_SIZE     10
#define ACK_RESPONSE_TIME   5
#define PACKET_SIZE         1
#define LOSS_PACKET_PROB    0.2

using namespace std;

void error(const char *);
void printWindow(list<unsigned>&);
void send_ACK_msg(list<unsigned>&);
bool contains(const list<unsigned>&, unsigned);

unsigned short window = MAX_WINDOW_SIZE;
unsigned initial_sequence = 1;
int sock;
size_t length;
ssize_t n;
socklen_t fromlen;
mutex window_lock;
mutex wait_lock;
condition_variable condition_var;
unsigned packetCounter = 0;
struct sockaddr_in server;
struct sockaddr_in from;

int main(int argc, char *argv[]) {
    char buf[HEADER_SIZE];
    list<unsigned> window_list;
    thread replier(send_ACK_msg, ref(window_list));
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
        if (packetCounter == 0){
            condition_var.notify_one();
        }
        TCP_Header tcp_header(buf);
        unsigned sequence_number = tcp_header.getSequence();
        cout << "Received a message. Sequence: " << sequence_number << endl;

        window_lock.lock();
        packetCounter++;
        // If the message has already been received then it is discarded.
        if (!contains(window_list, sequence_number)) {
            window_list.push_back(sequence_number);
        }
        window_lock.unlock();
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
    // Lock to wait for a condition
    unique_lock<mutex> my_lock(wait_lock);
    srand (static_cast<unsigned int>(time(nullptr)));
    int randNumber;
    while (true) {
        condition_var.wait(my_lock);
        this_thread::sleep_for(chrono::seconds(ACK_RESPONSE_TIME)); // Wait 5 seconds
        window_lock.lock();
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
        printWindow(window_list);
        cout << "Next sequence number to request: " << ack_Number << endl;

        /**
         * Reduce the size of the window if received very few messages or
         * increase it otherwise.
         */
        if (window_list.size() < (2*window/3)){
            window /= 2;
            cout << "Reducing the window to half. Window: " << window << endl;
        } else if (window < MAX_WINDOW_SIZE){
            window++;
            cout << "Increasing window by one. Window: " << window << endl;
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
        packetCounter = 0;
        window_lock.unlock();

        randNumber = rand()%10;
        if (randNumber < (1- LOSS_PACKET_PROB)*10) {
            TCP_Header ack_header(0, ack_Number, window, true);
            char *message = ack_header.header_to_Array();
            n = sendto(sock, message, HEADER_SIZE, 0, (struct sockaddr *) &from, fromlen);
            if (n < 0)
                error("sendto");
            delete[] message;
            cout << "Sending ACK. ACK num: " << ack_Number << endl;
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

void printWindow(list<unsigned> &window_list){
    auto iterator = window_list.begin();
    cout << "Window: ";
    while (iterator != window_list.end()){
        cout << *iterator << "  ";
        iterator++;
    }
    cout << endl;
}