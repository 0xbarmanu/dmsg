// UDP hole punching example, server code
// Base UDP code stolen from http://www.abc.se/~m6695/udp.html
// By Oscar Rodriguez
// This code is public domain, but you're a complete lunatic
// if you plan to use this code in any real program.
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "common.h" 
#include "miniupnpc.h"
 

char tracer[64];
int  tracer_port;
char checker[64];
int  checker_port; 

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s, i, j, slen=sizeof(si_other);

    msg_t  msg, recv_msg;
    client_t client_info; // 10 clients. Notice that we're not doing any bound checking.
    int n = 0;
 
    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
 
    // si_me stores our local endpoint. Remember that this program
    // has to be run in a network with UDP endpoint previously known
    // and directly accessible by all clients. In simpler terms, the
    // server cannot be behind a NAT.
    memset((char *) &si_me, 0, sizeof(si_me));
    memset(&msg, 0, sizeof(msg));

    if (readTracerInfo(tracer, &tracer_port, checker, &checker_port) != OK)
    {
        diep("Failed to read ini file!");
    }  

    d_print("tracer:port= %s:%d %s:%d\n", tracer, tracer_port, checker, checker_port);
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(tracer_port);
    si_other.sin_addr.s_addr = inet_addr(tracer);
   
    // register Port using UPnP
    registerPort("10.0.0.1","60008","60008","udp","1800");

    msg.cmd = HELLO_PUNCHING;
    if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), slen)==-1)
        diep("sendto");

    // si_me.sin_family = AF_INET;
    // si_me.sin_port = htons(CLIENT_PORT);
    // si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    // if (bind(s, (struct sockaddr*)(&si_me), sizeof(si_me))==-1)
    //     diep("bind");
 
    while (1)
    {
        // When a new client sends a datagram...
        memset((char*) &recv_msg, 0, sizeof(recv_msg));
        if (recvfrom(s, &recv_msg, sizeof(recv_msg), 0, (struct sockaddr*)(&si_other), (socklen_t*)&slen)==-1)
            diep("recvfrom");

        //d_print("Receive packet ");

        switch (recv_msg.cmd) {
            case CHECK:
                    
                d_print("CHECK from %s:%d msg.command=%d\n", inet_ntoa(si_other.sin_addr), 
                            ntohs(si_other.sin_port), recv_msg.cmd);

                // confirm check ok
                recv_msg.cmd = CHECK_OK;
                if (sendto(s, &recv_msg, sizeof(recv_msg), 0, (struct sockaddr*)(&si_other), slen)==-1)
                    diep("sendto");

                d_print("Send CHECK_OK to %s:%d msg.cmd=%d idx=%d port=%d\n", inet_ntoa(si_other.sin_addr), 
                        ntohs(si_other.sin_port), recv_msg.cmd, recv_msg.idx, recv_msg.client_info.port);
                break;

            case ACK_HELLO:
                d_print("ACK_HELLO from %s:%d msg.command=%d\n", inet_ntoa(si_other.sin_addr), 
                            ntohs(si_other.sin_port), recv_msg.cmd);
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


        // And we go back to listening. Notice that since UDP has no notion
        // of connections, we can use the same socket to listen for data
        // from different clients.
        // break;
    }
 
    // Actually, we never reach this point...
    close(s);
    return 0;
}
