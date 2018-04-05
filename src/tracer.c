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
#include "tracer.h"

#define NCLIENT 100
 
char tracer[64];
char checker[64];
int  checker_port;
int  tracer_port;
client_t clients[NCLIENT]; // 10 clients. Notice that we're not doing any bound checking.

int findFreeIdx(int i)
{
    int temp = i;
    while (clients[i].port != 0) {
        i ++;
        if (i == NCLIENT) i = 0;
        if (i == temp) 
            return -1; // not found free index
    }

    return i;
}

void sendCheckCmd(int s, int idx, client_t client_info)
{
    struct  sockaddr_in  si_checker;
    msg_t   msg;
    int     slen=sizeof(si_checker);

    si_checker.sin_family = AF_INET;
    si_checker.sin_addr.s_addr = inet_addr(checker);
    si_checker.sin_port = htons(checker_port);

    msg.cmd = CHECK;
    msg.idx = idx;
    msg. client_info = client_info;
    if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_checker), slen)==-1)
        diep("sendto");

    d_print("Sent CHECK to checker at %s:%d msg.cmd=%d\n", inet_ntoa(si_checker.sin_addr), si_checker.sin_port, msg.cmd);

    // if (sendto(s, &client_info, sizeof(client_info), 0, (struct sockaddr*)(&si_checker), slen)==-1)
    //     diep("sendto");
}

int main(void)
{
    struct sockaddr_in si_me, si_other, si_reply;
    int s, i, j, slen=sizeof(si_other);
    msg_t  msg, sendmsg;
    int idx = 0;
 
  
    if (readTracerInfo(tracer, &tracer_port, checker, &checker_port) != OK)
    {
        diep("Failed to read ini file!");
    }  

    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
 
    // init
    memset((char *)&clients, 0, sizeof(client_t)*NCLIENT);
    
    // si_me stores our local endpoint. Remember that this program
    // has to be run in a network with UDP endpoint previously known
    // and directly accessible by all clients. In simpler terms, the
    // server cannot be behind a NAT.
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(tracer_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)(&si_me), sizeof(si_me))==-1)
        diep("bind");


    while (1)
    {
        // When a new client sends a datagram...
        memset((char*) &msg, 0, sizeof(msg));
        if (recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), (socklen_t*)&slen)==-1)
            diep("recvfrom");

        d_print("Received packet from %s:%d msg.command=%d\n", inet_ntoa(si_other.sin_addr), si_other.sin_port, msg.cmd);
        
        switch (msg.cmd) {
            case HELLO_PUNCHING:
                msg.cmd = ACK_HELLO;
                si_reply.sin_family = AF_INET;
                si_reply.sin_port = si_other.sin_port;
                si_reply.sin_addr.s_addr = si_other.sin_addr.s_addr;
                if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_reply), slen)==-1)
                    diep("sendto");

                d_print("Send ACK_HELLO packet to %s:%d msg.command=%d\n", 
                        inet_ntoa(si_reply.sin_addr), si_reply.sin_port, msg.cmd);

                idx = findFreeIdx(idx);
                if (idx == -1)
                    diep("No more space for new client");

                clients[idx].host = si_other.sin_addr.s_addr;
                clients[idx].port = si_other.sin_port;

                sendCheckCmd(s, idx, clients[idx]);

                idx++;

            break;

            case CHECK_OK:
                si_other.sin_addr.s_addr = msg.client_info.host;
                d_print("Received CHECK_OK for client %s:%d idx=%d\n", inet_ntoa(si_other.sin_addr), msg.client_info.port, msg.idx);
            break;

            case HELLO_UPNP:

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

        d_print("Now we have %d clients\n", idx);
        // And we go back to listening. Notice that since UDP has no notion
        // of connections, we can use the same socket to listen for data
        // from different clients.
        // break;
    }
 
    // Actually, we never reach this point...
    close(s);
    return 0;
}