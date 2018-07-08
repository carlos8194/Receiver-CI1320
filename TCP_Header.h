//
// Created by carlos on 24/06/18.
//

#ifndef RECEIVER_TCP_HEADER_H
#define RECEIVER_TCP_HEADER_H

#define HEADER_SIZE     128

class TCP_Header {

public:
    explicit TCP_Header(char*);
    TCP_Header(unsigned seq, unsigned ACK, unsigned short wd, bool isACK);
    unsigned int getSequence() const;
    unsigned int getAck() const;
    unsigned short getWindow() const;
    bool IsACK() const;
    char* header_to_Array();

private:
    unsigned sequence;
    unsigned ack;
    unsigned short window;
    bool isACK;
};


#endif //RECEIVER_TCP_HEADER_H
