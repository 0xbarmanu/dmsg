#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
 


char tracer[64];
char checker[64];
int  tracer_port;
int  checker_port;


int main(void)
{
    struct sockaddr_in si_me, si_other, si_tracer;
    int s, i, j, slen=sizeof(si_other);
    msg_t  msg, sendmsg;
    client_t client_info;
 
    if (readTracerInfo(tracer, &tracer_port, checker, &checker_port) != OK)
    {
        diep("Failed to read ini file!");
    }  
    
    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
 
    // si_me stores our local endpoint. Remember that this program
    // has to be run in a network with UDP endpoint previously known
    // and directly accessible by all clients. In simpler terms, the
    // server cannot be behind a NAT.
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(checker_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)(&si_me), sizeof(si_me))==-1)
        diep("bind");

    memset((char *) &si_tracer, 0, sizeof(si_tracer));
    si_tracer.sin_family = AF_INET;
    si_tracer.sin_port = htons(tracer_port);
    si_tracer.sin_addr.s_addr = inet_addr(tracer);

    while (1)
    {
        // When a new client sends a datagram...
        memset((char*) &msg, 0, sizeof(msg));

        if (recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), (socklen_t*)&slen)==-1)
            diep("recvfrom");

        d_print("Received packet from %s:%d msg.cmd=%d idx=%d\n", inet_ntoa(si_other.sin_addr), si_other.sin_port, msg.cmd, msg.idx);
        
        switch (msg.cmd) {
            case CHECK:                
                // send CHECK to new client
                si_other.sin_addr.s_addr = msg.client_info.host;
                si_other.sin_port = msg.client_info.port;
                msg.cmd = CHECK;

                d_print("Received CHECK packet to check CLIENT=%s:%d\n", inet_ntoa(si_other.sin_addr), si_other.sin_port);

                if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), slen)==-1)
                    diep("sendto");

                d_print("Sent CHECK to %s:%d msg.cmd=%d idx=%d\n", inet_ntoa(si_other.sin_addr), si_other.sin_port, msg.cmd, msg.idx);

                break;

            case CHECK_OK:
                if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_tracer), slen)==-1)
                    diep("sendto");

                d_print("Sent CHECK_OK to %s:%d msg.cmd=%d idx=%d port=%d\n", inet_ntoa(si_tracer.sin_addr), 
                        si_tracer.sin_port, msg.cmd, msg.idx, msg.client_info.port);
                break;

            case LOCATION:
                break;

            default:
                break;
        }
        // The client's public UDP endpoint data is now in si_other.
        // Notice that we're completely ignoring the datagram payload.
        // If we want to support multiple clients inside the same NAT,
        // we'd have clients send their own private UDP endpoints
        // encoded in some way inside the payload, and store those as
        // well.
        // Now we add the client's UDP endpoint in our list.

        // And then tell everybody about everybody's public UDP endpoints

        // And we go back to listening. Notice that since UDP has no notion
        // of connections, we can use the same socket to listen for data
        // from different clients.
        // break;
    }
 
    // Actually, we never reach this point...
    close(s);
    return 0;
}