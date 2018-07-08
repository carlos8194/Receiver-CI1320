//
// Created by carlos on 24/06/18.
//

#include "TCP_Header.h"

TCP_Header::TCP_Header(char* str) {
// TODO llenar metodo
}

TCP_Header::TCP_Header(unsigned seq, unsigned ACK, unsigned short wd, bool isACK) {
    this->ack = ACK;
    this->isACK = isACK;
    this->sequence = seq;
    this->window = wd;
}

unsigned int TCP_Header::getSequence() const {
    return sequence;
}

unsigned int TCP_Header::getAck() const {
    return ack;
}

unsigned short TCP_Header::getWindow() const {
    return window;
}

bool TCP_Header::IsACK() const {
    return isACK;
}

char *TCP_Header::header_to_Array() {
    return nullptr;
    // TODO llenar metodo
}


